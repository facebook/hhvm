(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Core_kernel
open Typing_defs
open ServerCommandTypes.Extract_standalone
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (Syntax)
module SyntaxError = Full_fidelity_syntax_error
module SN = Naming_special_names
module Decl_provider = Decl_provider_ctx
module Class = Decl_provider.Class

exception NotFound

(* Internal error: for example, we are generating code for a dependency on an enum,
   but the passed dependency is not an enum *)
exception UnexpectedDependency

exception DependencyNotFound of string

exception Unsupported

let records_not_supported () = failwith "Records are not supported"

let value_exn ex opt =
  match opt with
  | Some s -> s
  | None -> raise ex

let value_or_not_found err_msg opt = value_exn (DependencyNotFound err_msg) opt

let get_class_exn ctx name =
  let not_found_msg = Printf.sprintf "Class %s" name in
  value_or_not_found not_found_msg @@ Decl_provider.get_class ctx name

let get_class_name : type a. a Typing_deps.Dep.variant -> string option =
 (* the OCaml compiler is not smart enough to let us use an or-pattern for all of these
  * because of how it's matching on a GADT *)
 fun dep ->
  Typing_deps.Dep.(
    match dep with
    | Const (cls, _) -> Some cls
    | Method (cls, _) -> Some cls
    | SMethod (cls, _) -> Some cls
    | Prop (cls, _) -> Some cls
    | SProp (cls, _) -> Some cls
    | Class cls -> Some cls
    | Cstr cls -> Some cls
    | AllMembers cls -> Some cls
    | Extends cls -> Some cls
    | Fun _
    | FunName _
    | GConst _
    | GConstName _ ->
      None
    | RecordDef _ -> records_not_supported ())

let global_dep_name dep =
  Typing_deps.Dep.(
    match dep with
    | Fun s
    | FunName s
    | Class s
    | GConst s
    | GConstName s ->
      s
    | Const (_, _)
    | Method (_, _)
    | SMethod (_, _)
    | Prop (_, _)
    | SProp (_, _)
    | Cstr _
    | AllMembers _
    | Extends _ ->
      raise UnexpectedDependency
    | RecordDef _ -> records_not_supported ())

let get_fun_pos ctx name =
  Decl_provider.get_fun ctx name |> Option.map ~f:(fun decl -> decl.fe_pos)

let get_class_pos ctx name =
  Decl_provider.get_class ctx name |> Option.map ~f:(fun decl -> Class.pos decl)

let get_typedef_pos ctx name =
  Decl_provider.get_typedef ctx name |> Option.map ~f:(fun decl -> decl.td_pos)

let get_gconst_pos ctx name =
  Decl_provider.get_gconst ctx name
  |> Option.map ~f:(fun ((r, _), _) -> Typing_reason.to_pos r)

let get_class_or_typedef_pos ctx name =
  Option.first_some (get_class_pos ctx name) (get_typedef_pos ctx name)

let get_dep_pos ctx dep =
  let open Typing_deps.Dep in
  match dep with
  | Fun name
  | FunName name ->
    get_fun_pos ctx name
  | Class name
  | Const (name, _)
  | Method (name, _)
  | SMethod (name, _)
  | Prop (name, _)
  | SProp (name, _)
  | Cstr name
  | AllMembers name
  | Extends name ->
    get_class_or_typedef_pos ctx name
  | GConst name
  | GConstName name ->
    get_gconst_pos ctx name
  | RecordDef _ -> records_not_supported ()

let make_mode_getter ~ctx ~get_pos ~find_in_file ~get_mode name =
  let open Option in
  get_pos ctx name >>= fun pos ->
  find_in_file (Pos.filename pos) name >>| get_mode

let get_fun_mode ctx =
  make_mode_getter
    ~ctx
    ~get_pos:get_fun_pos
    ~find_in_file:Ast_provider.find_fun_in_file
    ~get_mode:(fun fun_ -> fun_.Aast.f_mode)

let get_class_mode ctx =
  make_mode_getter
    ~ctx
    ~get_pos:get_class_pos
    ~find_in_file:Ast_provider.find_class_in_file
    ~get_mode:(fun class_ -> class_.Aast.c_mode)

let get_typedef_mode ctx =
  make_mode_getter
    ~ctx
    ~get_pos:get_typedef_pos
    ~find_in_file:Ast_provider.find_typedef_in_file
    ~get_mode:(fun typedef -> typedef.Aast.t_mode)

let get_gconst_mode ctx =
  make_mode_getter
    ~ctx
    ~get_pos:get_gconst_pos
    ~find_in_file:Ast_provider.find_gconst_in_file
    ~get_mode:(fun gconst -> gconst.Aast.cst_mode)

let get_class_or_typedef_mode ctx name =
  Option.first_some (get_class_mode ctx name) (get_typedef_mode ctx name)

let get_class_nast ctx name =
  let open Option in
  get_class_pos ctx name >>= fun pos ->
  Ast_provider.find_class_in_file (Pos.filename pos) name >>| Naming.class_

let get_dep_mode ctx dep =
  let open Typing_deps.Dep in
  match dep with
  | Fun name
  | FunName name ->
    get_fun_mode ctx name
  | Class name
  | Const (name, _)
  | Method (name, _)
  | SMethod (name, _)
  | Prop (name, _)
  | SProp (name, _)
  | Cstr name
  | AllMembers name
  | Extends name ->
    get_class_or_typedef_mode ctx name
  | GConst name
  | GConstName name ->
    get_gconst_mode ctx name
  | RecordDef _ -> records_not_supported ()

let is_strict_dep ctx dep = get_dep_mode ctx dep = Some FileInfo.Mstrict

let is_strict_fun ctx name = is_strict_dep ctx (Typing_deps.Dep.Fun name)

let is_strict_class ctx name = is_strict_dep ctx (Typing_deps.Dep.Class name)

let is_builtin_dep ctx dep =
  let msg = Typing_deps.Dep.to_string dep in
  let pos = value_or_not_found msg (get_dep_pos ctx dep) in
  Relative_path.prefix (Pos.filename pos) = Relative_path.Hhi

let is_relevant_dependency
    (target : target) (dep : Typing_deps.Dep.dependent Typing_deps.Dep.variant)
    =
  match target with
  | Function f -> dep = Typing_deps.Dep.Fun f || dep = Typing_deps.Dep.FunName f
  (* We have to collect dependencies of the entire class because dependency collection is
     coarse-grained: if cls's member depends on D, we get a dependency edge cls --> D,
     not (cls, member) --> D *)
  | Method (cls, _) -> get_class_name dep = Some cls

let get_filename ctx target =
  let pos =
    match target with
    | Function func ->
      let f = value_exn NotFound @@ Decl_provider.get_fun ctx func in
      Reason.to_pos (fst f.fe_type)
    | Method (cls, _) ->
      let cls = value_exn NotFound @@ Decl_provider.get_class ctx cls in
      Class.pos cls
  in
  Pos.filename pos

let extract_body ctx target =
  let filename = get_filename ctx target in
  let abs_filename = Relative_path.to_absolute filename in
  let file_content = In_channel.read_all abs_filename in
  Aast.(
    let pos =
      match target with
      | Function func ->
        let ast_function =
          value_exn NotFound @@ Ast_provider.find_fun_in_file filename func
        in
        ast_function.f_span
      | Method (cls, name) ->
        let ast_class =
          value_exn NotFound @@ Ast_provider.find_class_in_file filename cls
        in
        let m =
          value_exn NotFound
          @@ List.find ast_class.c_methods (fun meth -> snd meth.m_name = name)
        in
        m.m_span
    in
    Pos.get_text_from_pos file_content pos)

let print_error source_text error =
  let text =
    SyntaxError.to_positioned_string
      error
      (SourceText.offset_to_position source_text)
  in
  Hh_logger.log "%s\n" text

let tree_from_string s =
  let source_text = SourceText.make Relative_path.default s in
  let mode = Full_fidelity_parser.parse_mode source_text in
  let env = Full_fidelity_parser_env.make ?mode () in
  let tree = SyntaxTree.make ~env source_text in
  if List.is_empty (SyntaxTree.all_errors tree) then
    tree
  else (
    List.iter (SyntaxTree.all_errors tree) (print_error source_text);
    raise Hackfmt_error.InvalidSyntax
  )

let fixup_xhp =
  let re = Str.regexp "\\\\:" in
  Str.global_replace re ":"

let format text =
  try Libhackfmt.format_tree (tree_from_string (fixup_xhp text))
  with Hackfmt_error.InvalidSyntax -> text

let strip_ns obj_name =
  match String.rsplit2 obj_name '\\' with
  | Some (_, name) -> name
  | None -> obj_name

let list_items items = String.concat items ~sep:", "

let tparam_name (tp : Typing_defs.decl_tparam) = snd tp.tp_name

let function_make_default = "extract_standalone_make_default"

let call_make_default = Printf.sprintf "\\%s()" function_make_default

let print_fun_args ctx fun_type =
  let print_arg (pos : [ `standard of int | `variadic ]) arg =
    let name =
      match arg.fp_name with
      | Some n -> n
      | None -> ""
    in
    let ty =
      match snd arg.fp_type.et_type with
      | Typing_defs.Tany _ -> ""
      | _ ->
        Printf.sprintf "%s " @@ Typing_print.full_decl ctx arg.fp_type.et_type
    in
    match pos with
    | `standard index ->
      let inout =
        match arg.fp_kind with
        | FPinout -> "inout "
        | _ -> ""
      in
      let default =
        if index >= Typing_defs.arity_min fun_type.ft_arity then
          Printf.sprintf " = %s" call_make_default
        else
          ""
      in
      Printf.sprintf "%s%s%s%s" inout ty name default
    | `variadic -> Printf.sprintf "%s...%s" ty name
  in
  let args =
    List.mapi fun_type.ft_params ~f:(fun index arg ->
        print_arg (`standard index) arg)
  in
  let args =
    match fun_type.ft_arity with
    | Fvariadic (_, arg) -> args @ [print_arg `variadic arg]
    | Fellipsis _ -> args @ ["..."]
    | Fstandard _ -> args
  in
  String.concat ~sep:", " args

let print_constraint_kind = function
  | Ast_defs.Constraint_as -> "as"
  | Ast_defs.Constraint_super -> "super"
  | Ast_defs.Constraint_eq -> "="

let print_tparam_constraint ctx (ck, cty) =
  print_constraint_kind ck ^ " " ^ Typing_print.full_decl ctx cty

let print_tparam_variance = function
  | Ast_defs.Covariant -> "+"
  | Ast_defs.Contravariant -> "-"
  | Ast_defs.Invariant -> ""

let print_tparam
    ctx
    { tp_name = (_, name); tp_constraints = cstrl; tp_variance = variance; _ } =
  String.concat
    ~sep:" "
    ( (print_tparam_variance variance ^ name)
    :: List.map cstrl ~f:(print_tparam_constraint ctx) )

let rec get_reactivity_attr r =
  match r with
  | Nonreactive -> ""
  | Local _ -> "<<__RxLocal>> "
  | Shallow _ -> "<<__RxShallow>> "
  | Reactive _ -> "<<__Rx>> "
  | MaybeReactive r' -> get_reactivity_attr r'
  | RxVar (Some r') -> get_reactivity_attr r'
  | RxVar None -> ""

let get_function_declaration ctx fun_name fun_type =
  let tparams =
    match fun_type.ft_tparams with
    | ([], _) -> ""
    | (tparams, _) ->
      Printf.sprintf "<%s>"
      @@ list_items
      @@ List.map tparams ~f:(print_tparam ctx)
  in
  let args = print_fun_args ctx fun_type in
  let rtype =
    match snd fun_type.ft_ret.et_type with
    | Typing_defs.Tany _ -> ""
    | _ ->
      Printf.sprintf ": %s"
      @@ Typing_print.full_decl ctx fun_type.ft_ret.et_type
  in
  Printf.sprintf "function %s%s(%s)%s" (strip_ns fun_name) tparams args rtype

let get_init_for_prim = function
  | Aast_defs.Tnull -> "null"
  | Aast_defs.Tint
  | Aast_defs.Tnum ->
    "0"
  | Aast_defs.Tbool -> "false"
  | Aast_defs.Tfloat -> "0.0"
  | Aast_defs.Tstring
  | Aast_defs.Tarraykey ->
    "\"\""
  | Aast_defs.Tvoid
  | Aast_defs.Tresource
  | Aast_defs.Tnoreturn
  | Aast_defs.Tatom _ ->
    raise Unsupported

let get_enum_value ctx enum_name =
  let enum =
    value_exn UnexpectedDependency @@ Decl_provider.get_class ctx enum_name
  in
  if Class.kind enum <> Ast_defs.Cenum then
    raise Unsupported
  else
    Class.consts enum
    |> Sequence.map ~f:fst
    |> Sequence.find ~f:(fun name -> name <> "class")
    |> value_exn UnexpectedDependency

let get_init_from_type ctx ty =
  let open Typing_defs in
  let rec get_ (_, ty) =
    match ty with
    | Tprim prim -> get_init_for_prim prim
    | Toption _ -> "null"
    | Tarray _ -> "[]"
    | Tdarray _ -> "darray[]"
    | Tvarray _
    | Tvarray_or_darray _ ->
      "varray[]"
    | Ttuple elems ->
      Printf.sprintf "tuple(%s)" @@ list_items (List.map elems get_)
    (* Must be an enum, a containter type or a typedef for another supported type since those are the only
       cases we can have a constant value of some class *)
    | Tapply ((_, name), targs) ->
      (match name with
      | x
        when x = SN.Collections.cVec
             || x = SN.Collections.cKeyset
             || x = SN.Collections.cDict ->
        Printf.sprintf "%s[]" (strip_ns name)
      | x
        when x = SN.Collections.cVector
             || x = SN.Collections.cImmVector
             || x = SN.Collections.cMap
             || x = SN.Collections.cImmMap
             || x = SN.Collections.cSet
             || x = SN.Collections.cImmSet ->
        Printf.sprintf "%s {}" (strip_ns name)
      | x when x = SN.Collections.cPair ->
        (match targs with
        | [first; second] ->
          Printf.sprintf "Pair {%s, %s}" (get_ first) (get_ second)
        | _ -> raise UnexpectedDependency)
      | x when x = SN.Classes.cClassname ->
        (match targs with
        | [cls] -> Printf.sprintf "%s::class" (Typing_print.full_decl ctx cls)
        | _ -> raise Unsupported)
      | _ ->
        (match Decl_provider.get_class ctx name with
        | Some _ -> Printf.sprintf "%s::%s" name (get_enum_value ctx name)
        | None ->
          let typedef =
            value_exn UnexpectedDependency @@ Decl_provider.get_typedef ctx name
          in
          get_ typedef.td_type))
    | Tshape (_, fields) ->
      let print_shape_field name sft acc =
        (* Omit all optional fields *)
        if sft.sft_optional then
          acc
        else
          let name =
            match name with
            | Ast_defs.SFlit_int (_, s) -> s
            | Ast_defs.SFlit_str (_, s) -> Printf.sprintf "'%s'" s
            | Ast_defs.SFclass_const ((_, c), (_, s)) ->
              Printf.sprintf "%s::%s" c s
          in
          Printf.sprintf "%s => %s" name (get_ sft.sft_ty) :: acc
      in
      Printf.sprintf
        "shape(%s)"
        (list_items @@ Nast.ShapeMap.fold print_shape_field fields [])
    | _ -> raise Unsupported
  in
  get_ ty

let get_const_declaration ctx ?abstract:(is_abstract = false) ty name =
  let abstract =
    if is_abstract then
      "abstract "
    else
      ""
  in
  let typ = Typing_print.full_decl ctx ty in
  let init = get_init_from_type ctx ty in
  Printf.sprintf "%sconst %s %s = %s;" abstract typ name init

let get_global_object_declaration ctx obj =
  Typing_deps.Dep.(
    let description = to_string obj in
    match obj with
    | Fun f
    | FunName f ->
      let fun_type =
        value_or_not_found description @@ Decl_provider.get_fun ctx f
      in
      begin
        match fun_type with
        | { fe_type = (_, Tfun fun_type); _ } ->
          Printf.sprintf
            "%s%s{throw new \\Exception();}"
            (get_reactivity_attr fun_type.ft_reactive)
            (get_function_declaration ctx f fun_type)
        | _ -> failwith "Expected function type"
      end
    | GConst c
    | GConstName c ->
      let (const_type, _) =
        value_or_not_found description @@ Decl_provider.get_gconst ctx c
      in
      get_const_declaration ctx const_type (strip_ns c)
    (* No other global declarations *)
    | _ -> raise UnexpectedDependency)

let string_of_tprim = function
  | Tbool -> "bool"
  | Tint -> "int"
  | Tfloat -> "float"
  | Tnum -> "num"
  | Tstring -> "string"
  | Tarraykey -> "arraykey"
  | Tnull -> "null"
  | Tvoid -> "void"
  | Tresource -> "resource"
  | Tnoreturn -> "noreturn"
  | Tatom s -> ":@" ^ s

let rec string_of_hint hint =
  match snd hint with
  | Hoption hint -> "?" ^ string_of_hint hint
  | Hlike hint -> "~" ^ string_of_hint hint
  | Hfun
      {
        hf_reactive_kind = _;
        hf_is_coroutine;
        hf_param_tys;
        hf_param_kinds;
        hf_param_mutability = _;
        hf_variadic_ty;
        hf_return_ty;
        hf_is_mutable_return = _;
      } ->
    let coroutine =
      if hf_is_coroutine then
        "coroutine "
      else
        ""
    in
    let param_hints = List.map hf_param_tys ~f:string_of_hint in
    let param_kinds =
      List.map hf_param_kinds ~f:(function
          | Some Ast_defs.Pinout -> "inout "
          | None -> "")
    in
    let params = List.map2_exn param_kinds param_hints ~f:( ^ ) in
    let variadic =
      match hf_variadic_ty with
      | Some hint -> [string_of_hint hint ^ "..."]
      | None -> []
    in
    Printf.sprintf
      "(%sfunction(%s) : %s)"
      coroutine
      (String.concat ~sep:", " (params @ variadic))
      (string_of_hint hf_return_ty)
  | Htuple hints -> Printf.sprintf "(%s)" (string_of_hint_list hints)
  | Happly ((_, name), hints) ->
    let params =
      match hints with
      | [] -> ""
      | _ -> Printf.sprintf "<%s>" (string_of_hint_list hints)
    in
    name ^ params
  | Hshape { nsi_allows_unknown_fields; nsi_field_map } ->
    let string_of_shape_field { sfi_optional; sfi_name; sfi_hint } =
      let optional_prefix =
        if sfi_optional then
          "?"
        else
          ""
      in
      let name =
        match sfi_name with
        | Ast_defs.SFlit_int (_, str_i) -> str_i
        | Ast_defs.SFlit_str (_, str) -> "\"" ^ str ^ "\""
        | Ast_defs.SFclass_const ((_, class_name), (_, const_name)) ->
          class_name ^ "::" ^ const_name
      in
      optional_prefix ^ name ^ " => " ^ string_of_hint sfi_hint
    in
    let shape_fields = List.map nsi_field_map ~f:string_of_shape_field in
    let shape_suffix =
      if nsi_allows_unknown_fields then
        ["..."]
      else
        []
    in
    let shape_entries = shape_fields @ shape_suffix in
    Printf.sprintf "shape(%s)" (String.concat ~sep:", " shape_entries)
  | Haccess (root, ids) ->
    String.concat ~sep:"::" (string_of_hint root :: List.map ids ~f:snd)
  | Hsoft hint -> "@" ^ string_of_hint hint
  | Hmixed -> "mixed"
  | Hnonnull -> "nonnull"
  | Habstr s -> s
  | Harray (None, None) -> "array"
  | Harray (None, Some vhint) ->
    Printf.sprintf "array<%s>" (string_of_hint vhint)
  | Harray (Some khint, Some vhint) ->
    Printf.sprintf "array<%s, %s>" (string_of_hint khint) (string_of_hint vhint)
  | Harray (Some _, None) ->
    failwith "malformed type hint: Harray (Some _, None)"
  | Hdarray (khint, vhint) ->
    Printf.sprintf
      "darray<%s, %s>"
      (string_of_hint khint)
      (string_of_hint vhint)
  | Hvarray hint -> Printf.sprintf "varray<%s>" (string_of_hint hint)
  | Hvarray_or_darray (None, vhint) ->
    Printf.sprintf "varray_or_darray<%s>" (string_of_hint vhint)
  | Hvarray_or_darray (Some khint, vhint) ->
    Printf.sprintf
      "varray_or_darray<%s, %s>"
      (string_of_hint khint)
      (string_of_hint vhint)
  | Hprim prim -> string_of_tprim prim
  | Hthis -> "this"
  | Hdynamic -> "dynamic"
  | Hnothing -> "nothing"
  | Hpu_access (hint, (_, id)) -> string_of_hint hint ^ ":@" ^ id
  | Hunion hints -> Printf.sprintf "(%s)" (string_of_hint_list ~sep:" | " hints)
  | Hintersection hints ->
    Printf.sprintf "(%s)" (string_of_hint_list ~sep:" & " hints)
  | Hany -> failwith "unprintable type hint: Hany"
  | Herr -> failwith "unprintable type hint: Herr"

and string_of_hint_list ?(sep = ", ") hints =
  String.concat ~sep (List.map hints ~f:string_of_hint)

let get_class_declaration ctx (cls : Decl_provider.class_decl) =
  Decl_provider.(
    let consistent_construct =
      match Class.construct cls with
      | (_, ConsistentConstruct) -> "<<__ConsistentConstruct>> "
      | (_, (Inconsistent | FinalClass)) -> ""
    in
    let final =
      if Class.final cls then
        "final "
      else
        ""
    in
    let kind =
      match Class.kind cls with
      | Ast_defs.Cabstract -> "abstract class"
      | Ast_defs.Cnormal -> "class"
      | Ast_defs.Cinterface -> "interface"
      | Ast_defs.Ctrait -> "trait"
      | Ast_defs.Cenum -> "enum"
    in
    let name = Class.name cls in
    let tparams =
      if List.is_empty @@ Class.tparams cls then
        ""
      else
        Printf.sprintf
          "<%s>"
          (list_items @@ List.map (Class.tparams cls) ~f:(print_tparam ctx))
    in
    let nast = value_or_not_found name @@ get_class_nast ctx name in
    let prefix_if_nonempty prefix s =
      if s = "" then
        ""
      else
        prefix ^ s
    in
    let extends =
      prefix_if_nonempty " extends "
      @@ list_items
      @@ List.map nast.c_extends ~f:string_of_hint
    in
    let implements =
      prefix_if_nonempty " implements "
      @@ list_items
      @@ List.map nast.c_implements ~f:string_of_hint
    in
    (* TODO: traits, records *)
    Printf.sprintf
      "%s%s%s %s%s%s%s"
      consistent_construct
      final
      kind
      (strip_ns name)
      tparams
      extends
      implements)

let get_method_declaration
    ctx
    (meth : Typing_defs.class_elt)
    ?from_interface:(no_declare_abstract = false)
    ~is_static
    method_name =
  let abstract =
    if (not meth.ce_abstract) || no_declare_abstract then
      ""
    else
      "abstract "
  in
  let visibility = Typing_utils.string_of_visibility meth.ce_visibility in
  let static =
    if is_static then
      "static "
    else
      ""
  in
  let method_type =
    match Lazy.force meth.ce_type with
    | (_, Typing_defs.Tfun f) -> f
    | _ -> raise UnexpectedDependency
  in
  Printf.sprintf
    "%s%s%s %s%s"
    (get_reactivity_attr method_type.ft_reactive)
    abstract
    visibility
    static
    (get_function_declaration ctx method_name method_type)

let get_property_declaration ctx (prop : Typing_defs.class_elt) ~is_static name
    =
  let visibility = Typing_utils.string_of_visibility prop.ce_visibility in
  let static =
    if is_static then
      "static "
    else
      ""
  in
  let ty = Lazy.force prop.ce_type in
  let prop_type = Typing_print.full_decl ctx ty in
  let init =
    if is_static && not prop.ce_abstract then
      Printf.sprintf " = %s" (get_init_from_type ctx ty)
    else
      ""
  in
  Printf.sprintf "%s %s%s $%s%s;" visibility static prop_type name init

let get_typeconst_declaration ctx tconst name =
  let abstract =
    match tconst.ttc_abstract with
    | Typing_defs.TCAbstract _ -> "abstract "
    | TCPartiallyAbstract
    | TCConcrete ->
      ""
  in
  let typ =
    match tconst.ttc_type with
    | Some t -> Printf.sprintf " = %s" (Typing_print.full_decl ctx t)
    | None -> ""
  in
  let constr =
    match tconst.ttc_constraint with
    | Some t -> Printf.sprintf " as %s" (Typing_print.full_decl ctx t)
    | None -> ""
  in
  Printf.sprintf "%s const type %s%s%s;" abstract name constr typ

let get_class_elt_declaration
    ctx
    (cls : Decl_provider.class_decl)
    target
    (class_elt : 'a Typing_deps.Dep.variant) =
  Typing_deps.Dep.(
    Decl_provider.(
      let from_interface = Class.kind cls = Ast_defs.Cinterface in
      let description = to_string class_elt in
      match class_elt with
      | Const (_, const_name) ->
        if Class.has_typeconst cls const_name then
          let tconst =
            value_or_not_found description @@ Class.get_typeconst cls const_name
          in
          Some (get_typeconst_declaration ctx tconst const_name)
        else
          let cons =
            value_or_not_found description @@ Class.get_const cls const_name
          in
          Some
            (get_const_declaration
               ctx
               ~abstract:cons.cc_abstract
               cons.cc_type
               const_name)
      | Method (class_name, method_name) ->
        (match target with
        | ServerCommandTypes.Extract_standalone.Method
            (target_class_name, target_method_name)
          when class_name = target_class_name
               && method_name = target_method_name ->
          None
        | _ ->
          let m =
            value_or_not_found description @@ Class.get_method cls method_name
          in
          let decl =
            get_method_declaration
              ctx
              m
              ~from_interface
              ~is_static:false
              method_name
          in
          let body =
            if m.ce_abstract then
              ";"
            else
              "{throw new \\Exception();}"
          in
          Some (decl ^ body))
      | SMethod (class_name, smethod_name) ->
        (match target with
        | ServerCommandTypes.Extract_standalone.Method
            (target_class_name, target_method_name)
          when class_name = target_class_name
               && smethod_name = target_method_name ->
          None
        | _ ->
          let m =
            value_or_not_found description @@ Class.get_smethod cls smethod_name
          in
          let decl =
            get_method_declaration
              ctx
              m
              ~from_interface
              ~is_static:true
              smethod_name
          in
          let body =
            if m.ce_abstract then
              ";"
            else
              "{throw new \\Exception();}"
          in
          Some (decl ^ body))
      | Prop (_, prop_name) ->
        let p =
          value_or_not_found description @@ Class.get_prop cls prop_name
        in
        Some (get_property_declaration ctx p ~is_static:false prop_name)
      | SProp (_, sprop_name) ->
        let sp =
          value_or_not_found description @@ Class.get_sprop cls sprop_name
        in
        Some
          (get_property_declaration
             ctx
             sp
             ~is_static:true
             (String.lstrip ~drop:(fun c -> c = '$') sprop_name))
      (* Constructor should've been tackled earlier, and all other dependencies aren't class elements *)
      | Cstr _
      | Extends _
      | AllMembers _
      | Class _
      | Fun _
      | FunName _
      | GConst _
      | GConstName _ ->
        raise UnexpectedDependency
      | RecordDef _ -> records_not_supported ()))

let construct_enum ctx enum fields =
  let enum_name = Decl_provider.Class.name enum in
  let enum_type =
    value_exn UnexpectedDependency @@ Decl_provider.Class.enum_type enum
  in
  let string_enum_const = function
    | Typing_deps.Dep.Const (_, name) when name <> "class" ->
      Some
        (Printf.sprintf
           "%s = %s;"
           name
           (get_init_from_type ctx enum_type.te_base))
    | _ -> None
  in
  let base = Typing_print.full_decl ctx enum_type.te_base in
  let cons =
    match enum_type.te_constraint with
    | Some c -> " as " ^ Typing_print.full_decl ctx c
    | None -> ""
  in
  let enum_decl =
    Printf.sprintf "enum %s: %s%s" (strip_ns enum_name) base cons
  in
  let constants = List.filter_map fields ~f:string_enum_const in
  Printf.sprintf "%s {%s}" enum_decl (String.concat ~sep:"\n" constants)

let get_constructor_declaration tcopt cls prop_names =
  let (cstr, _) = Class.construct cls in
  match cstr with
  | None ->
    if List.is_empty prop_names then
      None
    else
      Some "public function __construct() {throw new \\Exception();}"
  | Some cstr ->
    let from_interface = Class.kind cls = Ast_defs.Cinterface in
    let decl =
      get_method_declaration
        tcopt
        cstr
        ~is_static:false
        ~from_interface
        "__construct"
    in
    let body =
      (* An interface may inherit a non-abstract constructor
         through a `require extends` declaration. *)
      if cstr.ce_abstract || from_interface then
        ";"
      else
        "{throw new \\Exception();}"
    in
    Some (decl ^ body)

let get_class_body ctx cls target class_elts =
  let name = Decl_provider.Class.name cls in
  let nast = value_or_not_found name @@ get_class_nast ctx name in
  let uses =
    List.map nast.c_uses ~f:(fun s ->
        Printf.sprintf "use %s;" (string_of_hint s))
  in
  let (req_extends, req_implements) =
    List.partition_map nast.c_reqs ~f:(fun (s, extends) ->
        if extends then
          `Fst (Printf.sprintf "require extends %s;" (string_of_hint s))
        else
          `Snd (Printf.sprintf "require implements %s;" (string_of_hint s)))
  in
  let open Typing_deps in
  let prop_names =
    List.filter_map class_elts ~f:(function
        | Dep.Prop (_, p) -> Some p
        | _ -> None)
  in
  let body =
    List.filter_map class_elts ~f:(function
        | Dep.AllMembers _
        | Dep.Extends _ ->
          raise UnexpectedDependency
        (* Constructor needs special treatment because we need
           information about properties. *)
        | Dep.Cstr _ -> get_constructor_declaration ctx cls prop_names
        | Dep.Const (_, "class") -> None
        | class_elt -> get_class_elt_declaration ctx cls target class_elt)
  in
  (* If we are extracting a method of this class, we should declare it
     here, with stubs of other class elements. *)
  let extracted_method =
    match target with
    | Method (cls_name, _) when cls_name = Decl_provider.Class.name cls ->
      [extract_body ctx target]
    | _ -> []
  in
  String.concat
    ~sep:"\n"
    (req_extends @ req_implements @ uses @ body @ extracted_method)

let construct_class ctx cls target fields =
  (* Enum declaration have a very different format: for example, no 'const' keyword
     for values, which are actually just class constants *)
  if Class.kind cls = Ast_defs.Cenum then
    construct_enum ctx cls fields
  else
    let decl = get_class_declaration ctx cls in
    let body = get_class_body ctx cls target fields in
    Printf.sprintf "%s {%s}" decl body

let construct_typedef ctx t =
  let not_found_msg = Printf.sprintf "typedef %s" t in
  let td =
    value_or_not_found not_found_msg @@ Decl_provider.get_typedef ctx t
  in
  let typ =
    if td.td_vis = Aast_defs.Transparent then
      "type"
    else
      "newtype"
  in
  let tparams =
    if List.is_empty td.td_tparams then
      ""
    else
      Printf.sprintf "<%s>" (list_items @@ List.map td.td_tparams tparam_name)
  in
  let constraint_ =
    match td.td_constraint with
    | Some ty -> " as " ^ Typing_print.full_decl ctx ty
    | None -> ""
  in
  Printf.sprintf
    "%s %s%s%s = %s;"
    typ
    (strip_ns t)
    tparams
    constraint_
    (Typing_print.full_decl ctx td.td_type)

let construct_type_declaration ctx t target fields =
  match Decl_provider.get_class ctx t with
  | Some cls -> construct_class ctx cls target fields
  | None -> construct_typedef ctx t

let rec do_add_dep ctx deps dep =
  if
    dep <> Typing_deps.Dep.Class SN.Typehints.wildcard
    && (not (HashSet.mem deps dep))
    && not (is_builtin_dep ctx dep)
  then (
    HashSet.add deps dep;
    add_signature_dependencies ctx deps dep
  )

and add_dep ctx deps ~this ty : unit =
  let visitor =
    object
      inherit [unit] Type_visitor.decl_type_visitor

      method! on_tapply _ _ (_, name) tyl =
        let dep = Typing_deps.Dep.Class name in
        do_add_dep ctx deps dep;

        (* If we have a constant of a generic type, it can only be an
           array type, e.g., vec<A>, for which don't need values of A
           to generate an initializer. *)
        List.iter tyl ~f:(add_dep ctx deps ~this);

        (* When adding a dependency on an enum, also add dependencies
           on all its values.  We need all the values and not only the
           ones used by the extracted code.  See the \with_switch test
           case. *)
        match Decl_provider.get_class ctx name with
        | Some enum when Class.kind enum = Ast_defs.Cenum ->
          Class.consts enum
          |> Sequence.iter ~f:(fun (const_name, _) ->
                 if const_name <> "class" then
                   do_add_dep
                     ctx
                     deps
                     (Typing_deps.Dep.Const (name, const_name)))
        | _ -> ()

      method! on_tshape _ _ _ fdm =
        Nast.ShapeMap.iter
          (fun name { sft_ty; _ } ->
            (match name with
            | Ast_defs.SFlit_int _
            | Ast_defs.SFlit_str _ ->
              ()
            | Ast_defs.SFclass_const ((_, c), (_, s)) ->
              do_add_dep ctx deps (Typing_deps.Dep.Class c);
              do_add_dep ctx deps (Typing_deps.Dep.Const (c, s)));
            add_dep ctx deps ~this sft_ty)
          fdm

      method! on_taccess () r ((_, root), tconsts) =
        let expand_type_access class_name tconsts =
          match tconsts with
          | [] -> raise UnexpectedDependency
          (* Expand Class::TConst1::TConst2[::...]: get TConst1 in
             Class, get its type or upper bound T, continue adding
             dependencies of T::TConst2[::...] *)
          | (_, tconst) :: tconsts ->
            do_add_dep ctx deps (Typing_deps.Dep.Const (class_name, tconst));
            let cls = get_class_exn ctx class_name in
            (match Decl_provider.Class.get_typeconst cls tconst with
            | Some typeconst ->
              Option.iter
                typeconst.ttc_type
                ~f:(add_dep ctx ~this:(Some class_name) deps);
              if not (List.is_empty tconsts) then (
                match (typeconst.ttc_type, typeconst.ttc_constraint) with
                | (Some tc_type, _)
                | (None, Some tc_type) ->
                  (* What does 'this' refer to inside of T? *)
                  let this =
                    match snd tc_type with
                    | Tapply ((_, name), _) -> Some name
                    | _ -> this
                  in
                  let taccess = Typing_defs.Taccess (tc_type, tconsts) in
                  add_dep ctx ~this deps (Typing_reason.Rnone, taccess)
                | (None, None) -> ()
              )
            | None -> ())
        in
        match root with
        | Taccess (root', tconsts') ->
          add_dep ctx ~this deps (r, Taccess (root', tconsts' @ tconsts))
        | Tapply ((_, name), _) -> expand_type_access name tconsts
        | Tthis -> expand_type_access (Option.value_exn this) tconsts
        | _ -> raise UnexpectedDependency
    end
  in
  visitor#on_type () ty

and add_signature_dependencies ctx deps obj =
  let open Typing_deps.Dep in
  let description = to_string obj in
  match get_class_name obj with
  | Some cls_name ->
    do_add_dep ctx deps (Typing_deps.Dep.Class cls_name);
    (match Decl_provider.get_class ctx cls_name with
    | None ->
      let td =
        value_or_not_found description @@ Decl_provider.get_typedef ctx cls_name
      in
      add_dep ctx ~this:None deps td.td_type;
      Option.iter td.td_constraint ~f:(add_dep ctx ~this:None deps)
    | Some cls ->
      let add_dep = add_dep ctx deps ~this:(Some cls_name) in
      (match obj with
      | Prop (_, name) ->
        let p = value_or_not_found description @@ Class.get_prop cls name in
        add_dep @@ Lazy.force p.ce_type;

        (* We need to initialize properties in the constructor, add a dependency on it *)
        do_add_dep ctx deps (Cstr cls_name)
      | SProp (_, name) ->
        let sp = value_or_not_found description @@ Class.get_sprop cls name in
        add_dep @@ Lazy.force sp.ce_type
      | Method (_, name) ->
        let m = value_or_not_found description @@ Class.get_method cls name in
        add_dep @@ Lazy.force m.ce_type
      | SMethod (_, name) ->
        (match Class.get_smethod cls name with
        | Some sm -> add_dep @@ Lazy.force sm.ce_type
        | None ->
          (match Class.get_method cls name with
          | Some _ ->
            HashSet.remove deps obj;
            do_add_dep ctx deps (Method (cls_name, name))
          | None -> raise (DependencyNotFound description)))
      | Const (_, name) ->
        (match Class.get_typeconst cls name with
        | Some tc ->
          if cls_name <> tc.ttc_origin then
            do_add_dep ctx deps (Const (tc.ttc_origin, name));
          Option.iter tc.ttc_type ~f:add_dep;
          Option.iter tc.ttc_constraint ~f:add_dep
        | None ->
          let c = value_or_not_found description @@ Class.get_const cls name in
          add_dep c.cc_type)
      | Cstr _ ->
        (match Class.construct cls with
        | (Some constr, _) -> add_dep @@ Lazy.force constr.ce_type
        | _ -> ())
      | Class _ ->
        Sequence.iter (Class.all_ancestors cls) (fun (_, ty) -> add_dep ty);
        Sequence.iter (Class.all_ancestor_reqs cls) (fun (_, ty) -> add_dep ty);
        Option.iter (Class.enum_type cls) ~f:(fun { te_base; te_constraint } ->
            add_dep te_base;
            Option.iter te_constraint ~f:add_dep)
      | AllMembers _ ->
        (* AllMembers is used for dependencies on enums, so we should depend on all constants *)
        Sequence.iter (Class.consts cls) (fun (name, c) ->
            if name <> "class" then add_dep c.cc_type)
      (* Ignore, we fetch class hierarchy when we call add_signature_dependencies on a class dep *)
      | Extends _ -> ()
      | _ -> raise UnexpectedDependency))
  | None ->
    (match obj with
    | Fun f
    | FunName f ->
      let func =
        value_or_not_found description @@ Decl_provider.get_fun ctx f
      in
      add_dep ctx ~this:None deps @@ func.fe_type
    | GConst c
    | GConstName c ->
      let (ty, _) =
        value_or_not_found description @@ Decl_provider.get_gconst ctx c
      in
      add_dep ctx ~this:None deps ty
    | _ -> raise UnexpectedDependency)

let get_implementation_dependencies ctx deps cls_name =
  let open Decl_provider in
  match get_class ctx cls_name with
  | None -> []
  | Some cls ->
    let open Typing_deps.Dep in
    let add_smethod_impl acc smethod_name =
      match Class.get_smethod cls smethod_name with
      | Some elt -> SMethod (elt.ce_origin, smethod_name) :: acc
      | _ -> acc
    in
    let add_method_impl acc method_name =
      match Class.get_method cls method_name with
      | Some elt -> Method (elt.ce_origin, method_name) :: acc
      | _ -> acc
    in
    let add_typeconst_impl acc typeconst_name =
      match Class.get_typeconst cls typeconst_name with
      | Some tc -> Const (tc.ttc_origin, typeconst_name) :: acc
      | _ -> acc
    in
    let add_const_impl acc const_name =
      match Class.get_const cls const_name with
      | Some c -> Const (c.cc_origin, const_name) :: acc
      | _ -> acc
    in
    let add_impls acc ancestor_name =
      let ancestor = get_class_exn ctx ancestor_name in
      if is_builtin_dep ctx (Class ancestor_name) then
        let acc =
          Sequence.fold
            (Class.smethods ancestor)
            ~init:acc
            ~f:(fun acc (smethod_name, _) -> add_smethod_impl acc smethod_name)
        in
        let acc =
          Sequence.fold
            (Class.methods ancestor)
            ~init:acc
            ~f:(fun acc (method_name, _) -> add_method_impl acc method_name)
        in
        let acc =
          Sequence.fold
            (Class.typeconsts ancestor)
            ~init:acc
            ~f:(fun acc (typeconst_name, _) ->
              add_typeconst_impl acc typeconst_name)
        in
        let acc =
          Sequence.fold
            (Class.consts ancestor)
            ~init:acc
            ~f:(fun acc (const_name, _) -> add_const_impl acc const_name)
        in
        acc
      else
        HashSet.fold
          (fun dep acc ->
            match dep with
            | SMethod (class_name, method_name) when class_name = ancestor_name
              ->
              add_smethod_impl acc method_name
            | Method (class_name, method_name) when class_name = ancestor_name
              ->
              add_method_impl acc method_name
            | Const (class_name, name) when class_name = ancestor_name ->
              if Option.is_some (Class.get_typeconst ancestor name) then
                add_typeconst_impl acc name
              else if Option.is_some (Class.get_const ancestor name) then
                add_const_impl acc name
              else
                acc
            | _ -> acc)
          deps
          acc
    in
    Sequence.fold
      (Sequence.append
         (Class.all_ancestor_names cls)
         (Class.all_ancestor_req_names cls))
      ~init:[]
      ~f:add_impls

let rec add_implementation_dependencies ctx deps =
  let open Typing_deps.Dep in
  let size = HashSet.length deps in
  HashSet.fold
    (fun dep acc ->
      match dep with
      | Class cls_name -> cls_name :: acc
      | _ -> acc)
    deps
    []
  |> List.concat_map ~f:(get_implementation_dependencies ctx deps)
  |> List.iter ~f:(do_add_dep ctx deps);
  if HashSet.length deps <> size then add_implementation_dependencies ctx deps

let get_dependency_origin ctx cls (dep : 'a Typing_deps.Dep.variant) =
  Decl_provider.(
    Typing_deps.Dep.(
      let description = to_string dep in
      let cls = value_or_not_found description @@ get_class ctx cls in
      match dep with
      | Prop (_, name) ->
        let p = value_or_not_found description @@ Class.get_prop cls name in
        p.ce_origin
      | SProp (_, name) ->
        let sp = value_or_not_found description @@ Class.get_sprop cls name in
        sp.ce_origin
      | Method (_, name) ->
        let m = value_or_not_found description @@ Class.get_method cls name in
        m.ce_origin
      | SMethod (_, name) ->
        let sm = value_or_not_found description @@ Class.get_smethod cls name in
        sm.ce_origin
      | Const (_, name) ->
        let c = value_or_not_found description @@ Class.get_const cls name in
        c.cc_origin
      | Cstr cls -> cls
      | _ -> raise UnexpectedDependency))

let get_dependencies ctx target =
  let dependencies = HashSet.create 0 in
  let add_dependency
      (root : Typing_deps.Dep.dependent Typing_deps.Dep.variant)
      (obj : Typing_deps.Dep.dependency Typing_deps.Dep.variant) : unit =
    if is_relevant_dependency target root then do_add_dep ctx dependencies obj
  in
  Typing_deps.add_dependency_callback "add_dependency" add_dependency;
  let filename = get_filename ctx target in
  (* Collect dependencies through side effects of typechecking and remove
   * the target function/method from the set of dependencies to avoid
   * declaring it twice.
   *)
  let () =
    Typing_deps.Dep.(
      match target with
      | Function func ->
        let (_ : (Tast.def * Typing_inference_env.t_global_with_pos) option) =
          Typing_check_service.type_fun ctx.Provider_context.tcopt filename func
        in
        HashSet.remove dependencies (Fun func);
        HashSet.remove dependencies (FunName func)
      | Method (cls, m) ->
        let (_
              : (Tast.def * Typing_inference_env.t_global_with_pos list) option)
            =
          Typing_check_service.type_class
            ctx.Provider_context.tcopt
            filename
            cls
        in
        HashSet.remove dependencies (Method (cls, m));
        HashSet.remove dependencies (SMethod (cls, m)))
  in
  add_implementation_dependencies ctx dependencies;
  HashSet.fold List.cons dependencies []

let group_class_dependencies_by_class ctx dependencies =
  List.fold_left
    dependencies
    ~f:(fun acc obj ->
      Typing_deps.Dep.(
        match obj with
        | Class cls ->
          if SMap.mem cls acc then
            acc
          else
            SMap.add cls [] acc
        | AllMembers _ -> acc
        | Extends _ -> acc
        | _ ->
          let cls = value_exn UnexpectedDependency (get_class_name obj) in
          let origin = get_dependency_origin ctx cls obj in
          (* Consider the following example:
           *
           * class Base {
           *   public static function do(): void {}
           * }
           * class Derived extends Base {}
           * function f(): void {
           *   Derived::do();
           * }
           *
           * We will pull both SMethod(Base, do) and SMethod(Derived, do) as
           * dependencies, but we should not generate method do() in Derived.
           * Therefore, we should ignore dependencies whose origin differs
           * from their class.
           *)
          if origin = cls then
            SMap.add cls [obj] acc ~combine:(fun x y -> y @ x)
          else
            acc))
    ~init:SMap.empty

(* Every namespace can contain declarations of classes, functions, constants
   as well as nested namespaces *)
type hack_namespace = {
  namespaces: (string, hack_namespace) Caml.Hashtbl.t;
  decls: string HashSet.t;
}

let subnamespace index name =
  let (nspaces, _) = String.rsplit2_exn ~on:'\\' name in
  if nspaces = "" then
    None
  else
    let nspaces = String.strip ~drop:(fun c -> c = '\\') nspaces in
    let nspaces = String.split ~on:'\\' nspaces in
    List.nth nspaces index

(* Build the recursive hack_namespace data structure for given declarations *)
let sort_by_namespace declarations =
  let rec add_decl nspace decl index =
    match subnamespace index decl with
    | Some name ->
      ( if Caml.Hashtbl.find_opt nspace.namespaces name = None then
        let nested = Caml.Hashtbl.create 0 in
        let declarations = HashSet.create 0 in
        Caml.Hashtbl.add
          nspace.namespaces
          name
          { namespaces = nested; decls = declarations } );
      add_decl (Caml.Hashtbl.find nspace.namespaces name) decl (index + 1)
    | None -> HashSet.add nspace.decls decl
  in
  let namespaces =
    { namespaces = Caml.Hashtbl.create 0; decls = HashSet.create 0 }
  in
  List.iter declarations ~f:(fun decl -> add_decl namespaces decl 0);
  namespaces

(* Takes declarations of Hack classes, functions, constants (map name -> code)
   and produces file(s) with Hack code:
    1) Groups declarations by namespaces, creating hack_namespace data structure
    2) Recursively prints the code in every namespace.
   Special case: since Hack files cannot contain both namespaces and toplevel
   declarations, we "generate" a separate file for toplevel declarations, using
   hh_single_type_check multifile syntax.
*)
let get_code strict_declarations partial_declarations =
  let get_code declarations =
    let decl_names = SMap.keys declarations in
    let global_namespace = sort_by_namespace decl_names in
    let code_from_namespace_decls name acc =
      Option.value (SMap.find_opt name declarations) ~default:[] @ acc
    in
    let toplevel =
      HashSet.fold code_from_namespace_decls global_namespace.decls []
    in
    let rec code_from_namespace nspace_name nspace_content code =
      let code = "}" :: code in
      let code =
        Caml.Hashtbl.fold code_from_namespace nspace_content.namespaces code
      in
      let code =
        HashSet.fold code_from_namespace_decls nspace_content.decls code
      in
      Printf.sprintf "namespace %s {" nspace_name :: code
    in
    let namespaces =
      Caml.Hashtbl.fold code_from_namespace global_namespace.namespaces []
    in
    (toplevel, namespaces)
  in
  let (strict_toplevel, strict_namespaces) = get_code strict_declarations in
  let (partial_toplevel, partial_namespaces) = get_code partial_declarations in
  let helper =
    Printf.sprintf
      "<<__Rx>> function %s(): nothing {throw new \\Exception();}"
      function_make_default
  in
  let strict_hh_prefix = "<?hh" in
  let partial_hh_prefix = "<?hh // partial" in
  let sections =
    [
      ( "//// strict_toplevel.php",
        (strict_hh_prefix, strict_toplevel @ [helper]) );
      ("//// partial_toplevel.php", (partial_hh_prefix, partial_toplevel));
      ("//// strict_namespaces.php", (strict_hh_prefix, strict_namespaces));
      ("//// partial_namespaces.php", (partial_hh_prefix, partial_namespaces));
    ]
  in
  let non_empty_sections =
    List.filter sections ~f:(fun (_, (_, decls)) -> not (List.is_empty decls))
  in
  let format_section (prefix, decls) =
    prefix ^ "\n" ^ format (String.concat ~sep:"\n" decls)
  in
  match non_empty_sections with
  | [(_, section)] -> format_section section
  | _ ->
    String.concat ~sep:"\n"
    @@ List.map non_empty_sections ~f:(fun (comment, section) ->
           comment ^ "\n" ^ format_section section)

let get_declarations ctx target class_dependencies global_dependencies =
  let (strict_class_dependencies, partial_class_dependencies) =
    SMap.partition (fun cls _ -> is_strict_class ctx cls) class_dependencies
  in
  let (strict_global_dependencies, partial_global_dependencies) =
    List.partition_tf global_dependencies ~f:(is_strict_dep ctx)
  in
  let add_declaration declarations name declaration =
    SMap.add name [declaration] declarations ~combine:(fun x y -> y @ x)
  in
  let add_global_declaration declarations dep =
    add_declaration
      declarations
      (global_dep_name dep)
      (get_global_object_declaration ctx dep)
  in
  let add_class_declaration cls fields declarations =
    add_declaration
      declarations
      cls
      (construct_type_declaration ctx cls target fields)
  in
  let strict_declarations =
    List.fold_left
      strict_global_dependencies
      ~f:add_global_declaration
      ~init:SMap.empty
    |> SMap.fold add_class_declaration strict_class_dependencies
  in
  let partial_declarations =
    List.fold_left
      partial_global_dependencies
      ~f:add_global_declaration
      ~init:SMap.empty
    |> SMap.fold add_class_declaration partial_class_dependencies
  in
  match target with
  | Function name ->
    let decl = extract_body ctx target in
    if is_strict_fun ctx name then
      (add_declaration strict_declarations name decl, partial_declarations)
    else
      (strict_declarations, add_declaration partial_declarations name decl)
  | Method _ -> (strict_declarations, partial_declarations)

let go ctx target =
  try
    let dependencies = get_dependencies ctx target in
    let (class_dependencies, global_dependencies) =
      List.partition_tf dependencies ~f:(fun dep ->
          Option.is_some (get_class_name dep))
    in
    let (strict_declarations, partial_declarations) =
      get_declarations
        ctx
        target
        (group_class_dependencies_by_class ctx class_dependencies)
        global_dependencies
    in
    get_code strict_declarations partial_declarations
  with
  | NotFound -> "Not found!"
  | DependencyNotFound d -> Printf.sprintf "Dependency not found: %s" d
  | Unsupported
  | UnexpectedDependency ->
    Printexc.get_backtrace ()

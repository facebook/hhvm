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
module Class = Decl_provider.Class

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

let get_fun_pos_exn ctx name = value_or_not_found name (get_fun_pos ctx name)

let get_class_pos ctx name =
  Decl_provider.get_class ctx name |> Option.map ~f:(fun decl -> Class.pos decl)

let get_class_pos_exn ctx name =
  value_or_not_found name (get_class_pos ctx name)

let get_typedef_pos ctx name =
  Decl_provider.get_typedef ctx name |> Option.map ~f:(fun decl -> decl.td_pos)

let get_gconst_pos ctx name =
  Decl_provider.get_gconst ctx name
  |> Option.map ~f:(fun (ty, _) -> Typing_defs.get_pos ty)

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

let make_nast_getter ~get_pos ~find_in_file ~naming =
  let nasts = ref SMap.empty in
  fun ctx name ->
    if SMap.mem name !nasts then
      Some (SMap.find name !nasts)
    else
      let open Option in
      get_pos ctx name >>= fun pos ->
      find_in_file ctx (Pos.filename pos) name >>= fun nast ->
      let nast = naming ctx nast in
      nasts := SMap.add name nast !nasts;
      Some nast

let get_fun_nast =
  make_nast_getter
    ~get_pos:get_fun_pos
    ~find_in_file:Ast_provider.find_fun_in_file
    ~naming:Naming.fun_

let get_fun_nast_exn ctx name = value_or_not_found name (get_fun_nast ctx name)

let get_class_nast =
  make_nast_getter
    ~get_pos:get_class_pos
    ~find_in_file:Ast_provider.find_class_in_file
    ~naming:Naming.class_

let get_typedef_nast =
  make_nast_getter
    ~get_pos:get_typedef_pos
    ~find_in_file:Ast_provider.find_typedef_in_file
    ~naming:Naming.typedef

let get_typedef_nast_exn ctx name =
  value_or_not_found name (get_typedef_nast ctx name)

let get_gconst_nast =
  make_nast_getter
    ~get_pos:get_gconst_pos
    ~find_in_file:Ast_provider.find_gconst_in_file
    ~naming:Naming.global_const

let get_gconst_nast_exn ctx name =
  value_or_not_found name (get_gconst_nast ctx name)

let make_class_element_nast_getter ~get_elements ~get_element_name =
  let elements_by_class_name = ref SMap.empty in
  fun ctx class_name element_name ->
    if SMap.mem class_name !elements_by_class_name then
      SMap.find_opt element_name (SMap.find class_name !elements_by_class_name)
    else
      let open Option in
      get_class_nast ctx class_name >>= fun class_ ->
      let elements_by_element_name =
        List.fold_left
          (get_elements class_)
          ~f:(fun elements element ->
            SMap.add (get_element_name element) element elements)
          ~init:SMap.empty
      in
      elements_by_class_name :=
        SMap.add class_name elements_by_element_name !elements_by_class_name;
      SMap.find_opt element_name elements_by_element_name

let get_method_nast =
  make_class_element_nast_getter
    ~get_elements:(fun class_ -> class_.c_methods)
    ~get_element_name:(fun method_ -> snd method_.m_name)

let get_method_nast_exn ctx class_name method_name =
  value_or_not_found
    (class_name ^ "::" ^ method_name)
    (get_method_nast ctx class_name method_name)

let get_const_nast =
  make_class_element_nast_getter
    ~get_elements:(fun class_ -> class_.c_consts)
    ~get_element_name:(fun const -> snd const.cc_id)

let get_typeconst_nast =
  make_class_element_nast_getter
    ~get_elements:(fun class_ -> class_.c_typeconsts)
    ~get_element_name:(fun typeconst -> snd typeconst.c_tconst_name)

let get_prop_nast =
  make_class_element_nast_getter
    ~get_elements:(fun class_ -> class_.c_vars)
    ~get_element_name:(fun class_var -> snd class_var.cv_id)

let get_prop_nast_exn ctx class_name prop_name =
  value_or_not_found
    (class_name ^ "::" ^ prop_name)
    (get_prop_nast ctx class_name prop_name)

let get_fun_mode ctx name =
  get_fun_nast ctx name |> Option.map ~f:(fun fun_ -> fun_.f_mode)

let get_class_mode ctx name =
  get_class_nast ctx name |> Option.map ~f:(fun class_ -> class_.c_mode)

let get_typedef_mode ctx name =
  get_typedef_nast ctx name |> Option.map ~f:(fun typedef -> typedef.t_mode)

let get_gconst_mode ctx name =
  get_gconst_nast ctx name |> Option.map ~f:(fun gconst -> gconst.cst_mode)

let get_class_or_typedef_mode ctx name =
  Option.first_some (get_class_mode ctx name) (get_typedef_mode ctx name)

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
  let msg = Typing_deps.Dep.variant_to_string dep in
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
    | Function name -> get_fun_pos_exn ctx name
    | Method (name, _) -> get_class_pos_exn ctx name
  in
  Pos.filename pos

let extract_target ctx target =
  let filename = get_filename ctx target in
  let abs_filename = Relative_path.to_absolute filename in
  let file_content = In_channel.read_all abs_filename in
  let pos =
    match target with
    | Function name ->
      let fun_ = get_fun_nast_exn ctx name in
      fun_.f_span
    | Method (class_name, method_name) ->
      let method_ = get_method_nast_exn ctx class_name method_name in
      method_.m_span
  in
  Pos.get_text_from_pos file_content pos

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

let concat_map ~sep ~f list = String.concat ~sep (List.map ~f list)

let function_make_default = "extract_standalone_make_default"

let call_make_default = Printf.sprintf "\\%s()" function_make_default

let extract_standalone_any = "EXTRACT_STANDALONE_ANY"

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

let string_of_shape_field_name = function
  | Ast_defs.SFlit_int (_, s) -> s
  | Ast_defs.SFlit_str (_, s) -> Printf.sprintf "'%s'" s
  | Ast_defs.SFclass_const ((_, c), (_, s)) -> Printf.sprintf "%s::%s" c s

let string_of_xhp_attr_info xhp_attr_info =
  match xhp_attr_info.xai_tag with
  | Some Required -> "@required"
  | Some LateInit -> "@lateinit"
  | None -> ""

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
  | Htuple hints ->
    Printf.sprintf "(%s)" (concat_map ~sep:", " ~f:string_of_hint hints)
  | Happly ((_, name), hints) ->
    let params =
      match hints with
      | [] -> ""
      | _ ->
        Printf.sprintf "<%s>" (concat_map ~sep:", " ~f:string_of_hint hints)
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
      Printf.sprintf
        "%s%s => %s"
        optional_prefix
        (string_of_shape_field_name sfi_name)
        (string_of_hint sfi_hint)
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
  | Hpu_access (hint, (_, id), _) -> string_of_hint hint ^ ":@" ^ id
  | Hunion hints ->
    Printf.sprintf "(%s)" (concat_map ~sep:" | " ~f:string_of_hint hints)
  | Hintersection hints ->
    Printf.sprintf "(%s)" (concat_map ~sep:" & " ~f:string_of_hint hints)
  | Hany -> extract_standalone_any
  | Herr -> extract_standalone_any

let maybe_string_of_user_attribute { ua_name; ua_params } =
  let name = snd ua_name in
  match ua_params with
  | [] when SMap.mem name SN.UserAttributes.as_map -> Some name
  | _ -> None

let string_of_user_attributes user_attributes =
  let user_attributes =
    List.filter_map ~f:maybe_string_of_user_attribute user_attributes
  in
  match user_attributes with
  | [] -> ""
  | _ -> Printf.sprintf "<<%s>>" (String.concat ~sep:", " user_attributes)

let string_of_variance = function
  | Ast_defs.Covariant -> "+"
  | Ast_defs.Contravariant -> "-"
  | Ast_defs.Invariant -> ""

let string_of_constraint (kind, hint) =
  let keyword =
    match kind with
    | Ast_defs.Constraint_as -> "as"
    | Ast_defs.Constraint_eq -> "="
    | Ast_defs.Constraint_super -> "super"
  in
  keyword ^ " " ^ string_of_hint hint

let string_of_tparam
    Aast.
      { tp_variance; tp_name; tp_constraints; tp_reified; tp_user_attributes } =
  let variance = string_of_variance tp_variance in
  let name = snd tp_name in
  let constraints = List.map tp_constraints ~f:string_of_constraint in
  let user_attributes = string_of_user_attributes tp_user_attributes in
  let reified =
    match tp_reified with
    | Erased -> ""
    | SoftReified
    | Reified ->
      "reify"
  in
  String.concat
    ~sep:" "
    (user_attributes :: reified :: (variance ^ name) :: constraints)

let string_of_tparams tparams =
  match tparams with
  | [] -> ""
  | _ ->
    Printf.sprintf "<%s>" (concat_map ~sep:", " ~f:string_of_tparam tparams)

let string_of_fun_param
    {
      param_type_hint;
      param_is_variadic;
      param_name;
      param_expr;
      param_callconv;
      param_user_attributes;
      _;
    } =
  let user_attributes = string_of_user_attributes param_user_attributes in
  let inout =
    match param_callconv with
    | Some Ast_defs.Pinout -> "inout"
    | None -> ""
  in
  let type_hint =
    match param_type_hint with
    | (_, Some hint) -> string_of_hint hint
    | (_, None) -> ""
  in
  let variadic =
    if param_is_variadic then
      "..."
    else
      ""
  in
  let default =
    match param_expr with
    | Some _ -> " = " ^ call_make_default
    | None -> ""
  in
  Printf.sprintf
    "%s %s %s %s%s%s"
    user_attributes
    inout
    type_hint
    variadic
    param_name
    default

let get_fun_declaration ctx name =
  let fun_ = get_fun_nast_exn ctx name in
  let user_attributes = string_of_user_attributes fun_.f_user_attributes in
  let tparams = string_of_tparams fun_.f_tparams in
  let variadic =
    match fun_.f_variadic with
    | FVvariadicArg fp -> [string_of_fun_param fp]
    | FVellipsis _ -> ["..."]
    | FVnonVariadic -> []
  in
  let params =
    String.concat
      ~sep:", "
      (List.map fun_.f_params ~f:string_of_fun_param @ variadic)
  in
  let ret =
    match fun_.f_ret with
    | (_, Some hint) -> ": " ^ string_of_hint hint
    | (_, None) -> ""
  in
  Printf.sprintf
    "%s function %s%s(%s)%s {throw new \\Exception();}"
    user_attributes
    (strip_ns name)
    tparams
    params
    ret

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
  | Aast_defs.Tatom s -> s
  | Aast_defs.Tvoid
  | Aast_defs.Tresource
  | Aast_defs.Tnoreturn ->
    raise Unsupported

let rec get_init_from_hint ctx tparams_stack hint =
  let unsupported_hint () =
    Hh_logger.log
      "%s: get_init_from_hint: unsupported hint: %s"
      (Pos.string (Pos.to_absolute (fst hint)))
      (Aast_defs.show_hint hint);
    raise Unsupported
  in
  match snd hint with
  | Hprim prim -> get_init_for_prim prim
  | Hoption _ -> "null"
  | Hlike hint -> get_init_from_hint ctx tparams_stack hint
  | Harray _ -> "darray[]"
  | Hdarray _ -> "darray[]"
  | Hvarray_or_darray _
  | Hvarray _ ->
    "varray[]"
  | Htuple hints ->
    Printf.sprintf
      "tuple(%s)"
      (concat_map ~sep:", " ~f:(get_init_from_hint ctx tparams_stack) hints)
  | Happly ((_, name), hints) ->
    (match () with
    | _
      when name = SN.Collections.cVec
           || name = SN.Collections.cKeyset
           || name = SN.Collections.cDict ->
      Printf.sprintf "%s[]" (strip_ns name)
    | _
      when name = SN.Collections.cVector
           || name = SN.Collections.cImmVector
           || name = SN.Collections.cMap
           || name = SN.Collections.cImmMap
           || name = SN.Collections.cSet
           || name = SN.Collections.cImmSet ->
      Printf.sprintf "%s {}" (strip_ns name)
    | _ when name = SN.Collections.cPair ->
      (match hints with
      | [first; second] ->
        Printf.sprintf
          "Pair {%s, %s}"
          (get_init_from_hint ctx tparams_stack first)
          (get_init_from_hint ctx tparams_stack second)
      | _ -> failwith "malformed hint")
    | _ when name = SN.Classes.cClassname ->
      (match hints with
      | [(_, Happly ((_, class_name), _))] ->
        Printf.sprintf "%s::class" class_name
      | _ -> raise UnexpectedDependency)
    | _ ->
      (match get_class_nast ctx name with
      | Some class_ ->
        if class_.c_kind = Ast_defs.Cenum then
          let const_name =
            match class_.c_consts with
            | [] -> failwith "empty enum"
            | const :: _ -> snd const.cc_id
          in
          Printf.sprintf "%s::%s" name const_name
        else
          unsupported_hint ()
      | None ->
        let typedef = get_typedef_nast_exn ctx name in
        let tparams =
          List.fold2_exn
            typedef.t_tparams
            hints
            ~init:SMap.empty
            ~f:(fun tparams tparam hint ->
              SMap.add (snd tparam.tp_name) hint tparams)
        in
        get_init_from_hint ctx (tparams :: tparams_stack) typedef.t_kind))
  | Hshape { nsi_field_map; _ } ->
    let non_optional_fields =
      List.filter nsi_field_map ~f:(fun shape_field_info ->
          not shape_field_info.sfi_optional)
    in
    let get_init_shape_field { sfi_hint; sfi_name; _ } =
      Printf.sprintf
        "%s => %s"
        (string_of_shape_field_name sfi_name)
        (get_init_from_hint ctx tparams_stack sfi_hint)
    in
    Printf.sprintf
      "shape(%s)"
      (concat_map ~sep:", " ~f:get_init_shape_field non_optional_fields)
  | Habstr name ->
    let rec loop tparams_stack =
      match tparams_stack with
      | tparams :: tparams_stack' ->
        (match SMap.find_opt name tparams with
        | Some hint -> get_init_from_hint ctx tparams_stack' hint
        | None -> loop tparams_stack')
      | [] -> unsupported_hint ()
    in
    loop tparams_stack
  | _ -> unsupported_hint ()

let get_init_from_hint ctx hint = get_init_from_hint ctx [] hint

let get_gconst_declaration ctx name =
  let gconst = get_gconst_nast_exn ctx name in
  let hint = value_or_not_found ("type of " ^ name) gconst.cst_type in
  let init = get_init_from_hint ctx hint in
  Printf.sprintf "const %s %s = %s;" (string_of_hint hint) (strip_ns name) init

let get_const_declaration ctx const =
  let name = snd const.cc_id in
  let abstract =
    match const.cc_expr with
    | Some _ -> ""
    | None -> "abstract"
  in
  let (type_, init) =
    match (const.cc_type, const.cc_expr) with
    | (Some hint, _) ->
      (string_of_hint hint, " = " ^ get_init_from_hint ctx hint)
    | (None, Some e) ->
      (match Decl_utils.infer_const e with
      | Some tprim ->
        let hint = (fst e, Hprim tprim) in
        ("", " = " ^ get_init_from_hint ctx hint)
      | None -> raise Unsupported)
    | (None, None) -> ("", "")
  in
  Printf.sprintf "%s const %s %s%s;" abstract type_ name init

let get_global_object_declaration ctx obj =
  Typing_deps.Dep.(
    match obj with
    | Fun f
    | FunName f ->
      get_fun_declaration ctx f
    | GConst c
    | GConstName c ->
      get_gconst_declaration ctx c
    (* No other global declarations *)
    | _ -> raise UnexpectedDependency)

let get_class_declaration class_ =
  let name = snd class_.c_name in
  let user_attributes = string_of_user_attributes class_.c_user_attributes in
  let final =
    if class_.c_final then
      "final"
    else
      ""
  in
  let kind =
    match class_.c_kind with
    | Ast_defs.Cabstract -> "abstract class"
    | Ast_defs.Cnormal -> "class"
    | Ast_defs.Cinterface -> "interface"
    | Ast_defs.Ctrait -> "trait"
    | Ast_defs.Cenum -> "enum"
  in
  let tparams = string_of_tparams class_.c_tparams.c_tparam_list in
  let extends =
    match class_.c_extends with
    | [] -> ""
    | _ ->
      Printf.sprintf
        "extends %s"
        (concat_map ~sep:", " ~f:string_of_hint class_.c_extends)
  in
  let implements =
    match class_.c_implements with
    | [] -> ""
    | _ ->
      Printf.sprintf
        "implements %s"
        (concat_map ~sep:", " ~f:string_of_hint class_.c_implements)
  in
  Printf.sprintf
    "%s %s %s %s%s %s %s"
    user_attributes
    final
    kind
    (strip_ns name)
    tparams
    extends
    implements

let get_method_declaration method_ ~from_interface =
  let abstract =
    if method_.m_abstract && not from_interface then
      "abstract"
    else
      ""
  in
  let final =
    if method_.m_final then
      "final"
    else
      ""
  in
  let visibility = string_of_visibility method_.m_visibility in
  let static =
    if method_.m_static then
      "static"
    else
      ""
  in
  let user_attributes = string_of_user_attributes method_.m_user_attributes in
  let name = strip_ns (snd method_.m_name) in
  let tparams = string_of_tparams method_.m_tparams in
  let variadic =
    match method_.m_variadic with
    | FVvariadicArg fp -> [string_of_fun_param fp]
    | FVellipsis _ -> ["..."]
    | FVnonVariadic -> []
  in
  let params =
    String.concat
      ~sep:", "
      (List.map method_.m_params ~f:string_of_fun_param @ variadic)
  in
  let ret =
    match method_.m_ret with
    | (_, Some hint) -> ": " ^ string_of_hint hint
    | (_, None) -> ""
  in
  let body =
    if method_.m_abstract || from_interface then
      ";"
    else
      "{throw new \\Exception();}"
  in
  Printf.sprintf
    "%s %s %s %s %s function %s%s(%s)%s%s"
    user_attributes
    abstract
    final
    visibility
    static
    name
    tparams
    params
    ret
    body

let get_prop_declaration ctx prop =
  let name = snd prop.cv_id in
  let user_attributes = string_of_user_attributes prop.cv_user_attributes in
  let (type_, init) =
    match (hint_of_type_hint prop.cv_type, prop.cv_expr) with
    | (Some hint, Some _) ->
      (string_of_hint hint, Printf.sprintf " = %s" (get_init_from_hint ctx hint))
    | (Some hint, None) -> (string_of_hint hint, "")
    | (None, None) -> ("", "")
    (* Untyped prop, not supported for now *)
    | (None, Some _) -> raise Unsupported
  in
  match prop.cv_xhp_attr with
  | None ->
    (* Ordinary property *)
    let visibility = string_of_visibility prop.cv_visibility in
    let static =
      if prop.cv_is_static then
        "static"
      else
        ""
    in
    Printf.sprintf
      "%s %s %s %s $%s%s;"
      user_attributes
      visibility
      static
      type_
      name
      init
  | Some xhp_attr_info ->
    (* XHP attribute *)
    Printf.sprintf
      "%s attribute %s %s %s %s;"
      user_attributes
      type_
      (String.lstrip ~drop:(fun c -> c = ':') name)
      init
      (string_of_xhp_attr_info xhp_attr_info)

let get_typeconst_declaration typeconst =
  let abstract =
    match typeconst.c_tconst_abstract with
    | TCAbstract _ -> "abstract"
    | TCPartiallyAbstract
    | TCConcrete ->
      ""
  in
  let name = snd typeconst.c_tconst_name in
  let type_ =
    match typeconst.c_tconst_type with
    | Some hint -> " = " ^ string_of_hint hint
    | None -> ""
  in
  let constraint_ =
    match typeconst.c_tconst_constraint with
    | Some hint -> " as " ^ string_of_hint hint
    | None -> ""
  in
  Printf.sprintf "%s const type %s%s%s;" abstract name constraint_ type_

let get_method_declaration ctx target class_name method_name =
  match target with
  | ServerCommandTypes.Extract_standalone.Method
      (target_class_name, target_method_name)
    when class_name = target_class_name && method_name = target_method_name ->
    None
  | _ ->
    let open Option in
    get_class_nast ctx class_name >>= fun class_ ->
    let from_interface = class_.c_kind = Ast_defs.Cinterface in
    get_method_nast ctx class_name method_name >>= fun method_ ->
    Some (get_method_declaration method_ ~from_interface)

let get_class_elt_declaration
    ctx target (class_elt : 'a Typing_deps.Dep.variant) =
  let open Typing_deps.Dep in
  match class_elt with
  | Const (class_name, const_name) ->
    (match get_typeconst_nast ctx class_name const_name with
    | Some typeconst -> Some (get_typeconst_declaration typeconst)
    | None ->
      (match get_const_nast ctx class_name const_name with
      | Some const -> Some (get_const_declaration ctx const)
      | None -> raise (DependencyNotFound (class_name ^ "::" ^ const_name))))
  | Method (class_name, method_name)
  | SMethod (class_name, method_name) ->
    get_method_declaration ctx target class_name method_name
  | Cstr class_name ->
    get_method_declaration ctx target class_name "__construct"
  | Prop (class_name, prop_name) ->
    let prop = get_prop_nast_exn ctx class_name prop_name in
    Some (get_prop_declaration ctx prop)
  | SProp (class_name, sprop_name) ->
    let sprop_name = String.lstrip ~drop:(fun c -> c = '$') sprop_name in
    let prop = get_prop_nast_exn ctx class_name sprop_name in
    Some (get_prop_declaration ctx prop)
  (* Constructor should've been tackled earlier, and all other dependencies aren't class elements *)
  | Extends _
  | AllMembers _
  | Class _
  | Fun _
  | FunName _
  | GConst _
  | GConstName _ ->
    raise UnexpectedDependency
  | RecordDef _ -> records_not_supported ()

let construct_enum ctx class_ =
  let name = snd class_.c_name in
  let enum =
    match class_.c_enum with
    | Some enum -> enum
    | None -> failwith ("not an enum: " ^ snd class_.c_name)
  in
  let constraint_ =
    match enum.e_constraint with
    | Some hint -> " as " ^ string_of_hint hint
    | None -> ""
  in
  let string_of_enum_const const =
    Printf.sprintf
      "%s = %s;"
      (snd const.cc_id)
      (get_init_from_hint ctx enum.e_base)
  in
  Printf.sprintf
    "enum %s: %s%s {%s}"
    (strip_ns name)
    (string_of_hint enum.e_base)
    constraint_
    (concat_map ~sep:"\n" ~f:string_of_enum_const class_.c_consts)

let get_class_body ctx class_ target class_elts =
  let name = snd class_.c_name in
  let uses =
    List.map class_.c_uses ~f:(fun s ->
        Printf.sprintf "use %s;" (string_of_hint s))
  in
  let (req_extends, req_implements) =
    List.partition_map class_.c_reqs ~f:(fun (s, extends) ->
        if extends then
          `Fst (Printf.sprintf "require extends %s;" (string_of_hint s))
        else
          `Snd (Printf.sprintf "require implements %s;" (string_of_hint s)))
  in
  let open Typing_deps in
  let body =
    List.filter_map class_elts ~f:(function
        | Dep.AllMembers _
        | Dep.Extends _ ->
          raise UnexpectedDependency
        | Dep.Const (_, "class") -> None
        | class_elt -> get_class_elt_declaration ctx target class_elt)
  in
  (* If we are extracting a method of this class, we should declare it
     here, with stubs of other class elements. *)
  let extracted_method =
    match target with
    | Method (cls_name, _) when cls_name = name -> [extract_target ctx target]
    | _ -> []
  in
  String.concat
    ~sep:"\n"
    (req_extends @ req_implements @ uses @ body @ extracted_method)

let construct_class ctx class_ target fields =
  let decl = get_class_declaration class_ in
  let body = get_class_body ctx class_ target fields in
  Printf.sprintf "%s {%s}" decl body

let construct_enum_or_class ctx class_ target fields =
  match class_.c_kind with
  | Ast_defs.Cabstract
  | Ast_defs.Cnormal
  | Ast_defs.Cinterface
  | Ast_defs.Ctrait ->
    construct_class ctx class_ target fields
  | Ast_defs.Cenum -> construct_enum ctx class_

let construct_typedef typedef =
  let name = snd typedef.t_name in
  let keyword =
    match typedef.t_vis with
    | Aast_defs.Transparent -> "type"
    | Aast_defs.Opaque -> "newtype"
  in
  let tparams = string_of_tparams typedef.t_tparams in
  let constraint_ =
    match typedef.t_constraint with
    | Some hint -> " as " ^ string_of_hint hint
    | None -> ""
  in
  let pos = fst typedef.t_name in
  let hh_fixmes =
    String.concat
      (List.map
         ~f:(fun code -> Printf.sprintf "/* HH_FIXME[%d] */\n" code)
         (ISet.elements (Fixme_provider.get_fixme_codes_for_pos pos)))
  in
  Printf.sprintf
    "%s%s %s%s%s = %s;"
    hh_fixmes
    keyword
    (strip_ns name)
    tparams
    constraint_
    (string_of_hint typedef.t_kind)

let construct_type_declaration ctx t target fields =
  match get_class_nast ctx t with
  | Some class_ -> construct_enum_or_class ctx class_ target fields
  | None ->
    let typedef = get_typedef_nast_exn ctx t in
    construct_typedef typedef

type extraction_env = {
  dependencies: Typing_deps.Dep.dependency Typing_deps.Dep.variant HashSet.t;
  depends_on_make_default: bool ref;
  depends_on_any: bool ref;
}

let rec do_add_dep ctx env dep =
  if
    dep <> Typing_deps.Dep.Class SN.Typehints.wildcard
    && (not (HashSet.mem env.dependencies dep))
    && not (is_builtin_dep ctx dep)
  then (
    HashSet.add env.dependencies dep;
    add_signature_dependencies ctx env dep
  )

and add_dep ctx env ~this ty : unit =
  let visitor =
    object
      inherit [unit] Type_visitor.decl_type_visitor as super

      method! on_tany _ _ = env.depends_on_any := true

      method! on_tfun () r ft =
        if List.length ft.ft_params > arity_min ft.ft_arity then
          env.depends_on_make_default := true;
        super#on_tfun () r ft

      method! on_tapply _ _ (_, name) tyl =
        let dep = Typing_deps.Dep.Class name in
        do_add_dep ctx env dep;

        (* If we have a constant of a generic type, it can only be an
           array type, e.g., vec<A>, for which don't need values of A
           to generate an initializer. *)
        List.iter tyl ~f:(add_dep ctx env ~this)

      method! on_tshape _ _ _ fdm =
        Nast.ShapeMap.iter
          (fun name { sft_ty; _ } ->
            (match name with
            | Ast_defs.SFlit_int _
            | Ast_defs.SFlit_str _ ->
              ()
            | Ast_defs.SFclass_const ((_, c), (_, s)) ->
              do_add_dep ctx env (Typing_deps.Dep.Class c);
              do_add_dep ctx env (Typing_deps.Dep.Const (c, s)));
            add_dep ctx env ~this sft_ty)
          fdm

      method! on_taccess () r (root, tconsts) =
        let expand_type_access class_name tconsts =
          match tconsts with
          | [] -> raise UnexpectedDependency
          (* Expand Class::TConst1::TConst2[::...]: get TConst1 in
             Class, get its type or upper bound T, continue adding
             dependencies of T::TConst2[::...] *)
          | (_, tconst) :: tconsts ->
            do_add_dep ctx env (Typing_deps.Dep.Const (class_name, tconst));
            let cls = get_class_exn ctx class_name in
            (match Decl_provider.Class.get_typeconst cls tconst with
            | Some typeconst ->
              Option.iter
                typeconst.ttc_type
                ~f:(add_dep ctx ~this:(Some class_name) env);
              if not (List.is_empty tconsts) then (
                match (typeconst.ttc_type, typeconst.ttc_constraint) with
                | (Some tc_type, _)
                | (None, Some tc_type) ->
                  (* What does 'this' refer to inside of T? *)
                  let this =
                    match Typing_defs.get_node tc_type with
                    | Tapply ((_, name), _) -> Some name
                    | _ -> this
                  in
                  let taccess = Typing_defs.Taccess (tc_type, tconsts) in
                  add_dep
                    ctx
                    ~this
                    env
                    (Typing_defs.mk (Typing_reason.Rnone, taccess))
                | (None, None) -> ()
              )
            | None -> ())
        in
        match Typing_defs.get_node root with
        | Taccess (root', tconsts') ->
          add_dep
            ctx
            ~this
            env
            (Typing_defs.mk (r, Taccess (root', tconsts' @ tconsts)))
        | Tapply ((_, name), _) -> expand_type_access name tconsts
        | Tthis -> expand_type_access (Option.value_exn this) tconsts
        | _ -> raise UnexpectedDependency
    end
  in
  visitor#on_type () ty

and add_signature_dependencies ctx env obj =
  let open Typing_deps.Dep in
  let description = variant_to_string obj in
  match get_class_name obj with
  | Some cls_name ->
    do_add_dep ctx env (Typing_deps.Dep.Class cls_name);
    (match Decl_provider.get_class ctx cls_name with
    | None ->
      let td =
        value_or_not_found description @@ Decl_provider.get_typedef ctx cls_name
      in
      add_dep ctx ~this:None env td.td_type;
      Option.iter td.td_constraint ~f:(add_dep ctx ~this:None env)
    | Some cls ->
      let add_dep = add_dep ctx env ~this:(Some cls_name) in
      (match obj with
      | Prop (_, name) ->
        let p = value_or_not_found description @@ Class.get_prop cls name in
        add_dep @@ Lazy.force p.ce_type;

        (* We need to initialize properties in the constructor, add a dependency on it *)
        do_add_dep ctx env (Cstr cls_name)
      | SProp (_, name) ->
        let sp = value_or_not_found description @@ Class.get_sprop cls name in
        add_dep @@ Lazy.force sp.ce_type
      | Method (_, name) ->
        let m = value_or_not_found description @@ Class.get_method cls name in
        add_dep @@ Lazy.force m.ce_type;
        Class.all_ancestor_names cls
        |> List.iter ~f:(fun ancestor_name ->
               match Decl_provider.get_class ctx ancestor_name with
               | Some ancestor when Class.has_method ancestor name ->
                 do_add_dep ctx env (Method (ancestor_name, name))
               | _ -> ())
      | SMethod (_, name) ->
        (match Class.get_smethod cls name with
        | Some sm ->
          add_dep @@ Lazy.force sm.ce_type;
          Class.all_ancestor_names cls
          |> List.iter ~f:(fun ancestor_name ->
                 match Decl_provider.get_class ctx ancestor_name with
                 | Some ancestor when Class.has_smethod ancestor name ->
                   do_add_dep ctx env (SMethod (ancestor_name, name))
                 | _ -> ())
        | None ->
          (match Class.get_method cls name with
          | Some _ ->
            HashSet.remove env.dependencies obj;
            do_add_dep ctx env (Method (cls_name, name))
          | None -> raise (DependencyNotFound description)))
      | Const (_, name) ->
        (match Class.get_typeconst cls name with
        | Some tc ->
          if cls_name <> tc.ttc_origin then
            do_add_dep ctx env (Const (tc.ttc_origin, name));
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
        List.iter (Class.all_ancestors cls) (fun (_, ty) -> add_dep ty);
        List.iter (Class.all_ancestor_reqs cls) (fun (_, ty) -> add_dep ty);
        Option.iter (Class.enum_type cls) ~f:(fun { te_base; te_constraint } ->
            add_dep te_base;
            Option.iter te_constraint ~f:add_dep)
      | AllMembers _ ->
        (* AllMembers is used for dependencies on enums, so we should depend on all constants *)
        List.iter (Class.consts cls) (fun (name, c) ->
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
      add_dep ctx ~this:None env @@ func.fe_type
    | GConst c
    | GConstName c ->
      let (ty, _) =
        value_or_not_found description @@ Decl_provider.get_gconst ctx c
      in
      add_dep ctx ~this:None env ty
    | _ -> raise UnexpectedDependency)

let get_implementation_dependencies ctx env cls_name =
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
          List.fold
            (Class.smethods ancestor)
            ~init:acc
            ~f:(fun acc (smethod_name, _) -> add_smethod_impl acc smethod_name)
        in
        let acc =
          List.fold
            (Class.methods ancestor)
            ~init:acc
            ~f:(fun acc (method_name, _) -> add_method_impl acc method_name)
        in
        let acc =
          List.fold
            (Class.typeconsts ancestor)
            ~init:acc
            ~f:(fun acc (typeconst_name, _) ->
              add_typeconst_impl acc typeconst_name)
        in
        let acc =
          List.fold
            (Class.consts ancestor)
            ~init:acc
            ~f:(fun acc (const_name, _) -> add_const_impl acc const_name)
        in
        acc
      else
        HashSet.fold env.dependencies ~init:acc ~f:(fun dep acc ->
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
    in
    let result =
      List.fold ~init:[] ~f:add_impls (Class.all_ancestor_names cls)
    in
    let result =
      Sequence.fold ~init:result ~f:add_impls (Class.all_ancestor_req_names cls)
    in
    result

let rec add_implementation_dependencies ctx env =
  let open Typing_deps.Dep in
  let size = HashSet.length env.dependencies in
  HashSet.fold env.dependencies ~init:[] ~f:(fun dep acc ->
      match dep with
      | Class cls_name -> cls_name :: acc
      | _ -> acc)
  |> List.concat_map ~f:(get_implementation_dependencies ctx env)
  |> List.iter ~f:(do_add_dep ctx env);
  if HashSet.length env.dependencies <> size then
    add_implementation_dependencies ctx env

let get_dependency_origin ctx cls (dep : 'a Typing_deps.Dep.variant) =
  Decl_provider.(
    Typing_deps.Dep.(
      let description = variant_to_string dep in
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

let collect_dependencies ctx target =
  let filename = get_filename ctx target in
  let env =
    {
      dependencies = HashSet.create ();
      depends_on_make_default = ref false;
      depends_on_any = ref false;
    }
  in
  let add_dependency
      (root : Typing_deps.Dep.dependent Typing_deps.Dep.variant)
      (obj : Typing_deps.Dep.dependency Typing_deps.Dep.variant) : unit =
    if is_relevant_dependency target root then do_add_dep ctx env obj
  in
  Typing_deps.add_dependency_callback "add_dependency" add_dependency;
  (* Collect dependencies through side effects of typechecking and remove
   * the target function/method from the set of dependencies to avoid
   * declaring it twice.
   *)
  let () =
    Typing_deps.Dep.(
      match target with
      | Function func ->
        let (_ : (Tast.def * Typing_inference_env.t_global_with_pos) option) =
          Typing_check_service.type_fun ctx filename func
        in
        add_implementation_dependencies ctx env;
        HashSet.remove env.dependencies (Fun func);
        HashSet.remove env.dependencies (FunName func)
      | Method (cls, m) ->
        let (_
              : (Tast.def * Typing_inference_env.t_global_with_pos list) option)
            =
          Typing_check_service.type_class ctx filename cls
        in
        HashSet.add env.dependencies (Method (cls, m));
        HashSet.add env.dependencies (SMethod (cls, m));
        add_implementation_dependencies ctx env;
        HashSet.remove env.dependencies (Method (cls, m));
        HashSet.remove env.dependencies (SMethod (cls, m)))
  in
  env

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
        let declarations = HashSet.create () in
        Caml.Hashtbl.add
          nspace.namespaces
          name
          { namespaces = nested; decls = declarations } );
      add_decl (Caml.Hashtbl.find nspace.namespaces name) decl (index + 1)
    | None -> HashSet.add nspace.decls decl
  in
  let namespaces =
    { namespaces = Caml.Hashtbl.create 0; decls = HashSet.create () }
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
let get_code
    ~depends_on_make_default
    ~depends_on_any
    strict_declarations
    partial_declarations =
  let get_code declarations =
    let decl_names = SMap.keys declarations in
    let global_namespace = sort_by_namespace decl_names in
    let code_from_namespace_decls name acc =
      Option.value (SMap.find_opt name declarations) ~default:[] @ acc
    in
    let toplevel =
      HashSet.fold global_namespace.decls ~init:[] ~f:code_from_namespace_decls
    in
    let rec code_from_namespace nspace_name nspace_content code =
      let code = "}" :: code in
      let code =
        Caml.Hashtbl.fold code_from_namespace nspace_content.namespaces code
      in
      let code =
        HashSet.fold
          nspace_content.decls
          ~init:code
          ~f:code_from_namespace_decls
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
  let helpers =
    ( if depends_on_make_default then
      [
        Printf.sprintf
          "<<__Rx>> function %s(): nothing {throw new \\Exception();}"
          function_make_default;
      ]
    else
      [] )
    @
    if depends_on_any then
      [
        "/* HH_FIXME[4101] */";
        Printf.sprintf
          "type %s = \\%s_;"
          extract_standalone_any
          extract_standalone_any;
        Printf.sprintf "type %s_<T> = T;" extract_standalone_any;
      ]
    else
      []
  in
  let strict_hh_prefix = "<?hh" in
  let partial_hh_prefix = "<?hh // partial" in
  let sections =
    [
      ("//// strict_toplevel.php", (strict_hh_prefix, strict_toplevel @ helpers));
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
    concat_map
      ~sep:"\n"
      ~f:(fun (comment, section) -> comment ^ "\n" ^ format_section section)
      non_empty_sections

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
    let decl = extract_target ctx target in
    if is_strict_fun ctx name then
      (add_declaration strict_declarations name decl, partial_declarations)
    else
      (strict_declarations, add_declaration partial_declarations name decl)
  | Method _ -> (strict_declarations, partial_declarations)

let go ctx target =
  try
    let env = collect_dependencies ctx target in
    let dependencies = HashSet.fold env.dependencies ~init:[] ~f:List.cons in
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
    get_code
      ~depends_on_make_default:!(env.depends_on_make_default)
      ~depends_on_any:!(env.depends_on_any)
      strict_declarations
      partial_declarations
  with
  | DependencyNotFound d -> Printf.sprintf "Dependency not found: %s" d
  | Unsupported
  | UnexpectedDependency ->
    Printexc.get_backtrace ()

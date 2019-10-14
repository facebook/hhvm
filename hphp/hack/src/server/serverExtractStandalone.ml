(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
open ServerCommandTypes.Extract_standalone
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (Syntax)
module SyntaxError = Full_fidelity_syntax_error
module SN = Naming_special_names

exception NotFound

(* Internal error: for example, we are generating code for a dependency on an enum,
   but the passed dependency is not an enum *)
exception UnexpectedDependency

exception DependencyNotFound of string

exception Unsupported

exception InvalidInput

let records_not_supported () = failwith "Records are not supported"

let value_exn ex opt =
  match opt with
  | Some s -> s
  | None -> raise ex

let value_or_not_found err_msg opt = value_exn (DependencyNotFound err_msg) opt

let get_class_exn name =
  let not_found_msg = Printf.sprintf "Class %s" name in
  value_or_not_found not_found_msg @@ Decl_provider.get_class name

let get_class_name (dep : Typing_deps.Dep.variant) =
  Typing_deps.Dep.(
    match dep with
    | Const (cls, _)
    | Method (cls, _)
    | SMethod (cls, _)
    | Prop (cls, _)
    | SProp (cls, _)
    | Cstr cls
    | Class cls
    | AllMembers cls
    | Extends cls ->
      Some cls
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

let get_fun_pos name =
  Decl_provider.get_fun name |> Option.map ~f:(fun decl -> decl.fe_pos)

let get_class_pos name =
  Decl_provider.get_class name
  |> Option.map ~f:(fun decl -> Decl_provider.Class.pos decl)

let get_typedef_pos name =
  Decl_provider.get_typedef name |> Option.map ~f:(fun decl -> decl.td_pos)

let get_gconst_pos name =
  Decl_provider.get_gconst name
  |> Option.map ~f:(fun ((r, _), _) -> Typing_reason.to_pos r)

let get_class_or_typedef_pos name =
  Option.first_some (get_class_pos name) (get_typedef_pos name)

let get_dep_pos dep =
  let open Typing_deps.Dep in
  match dep with
  | Fun name
  | FunName name ->
    get_fun_pos name
  | Class name
  | Const (name, _)
  | Method (name, _)
  | SMethod (name, _)
  | Prop (name, _)
  | SProp (name, _)
  | Cstr name
  | AllMembers name
  | Extends name ->
    get_class_or_typedef_pos name
  | GConst name
  | GConstName name ->
    get_gconst_pos name
  | RecordDef _ -> records_not_supported ()

let make_mode_getter ~get_pos ~find_in_file ~get_mode name =
  let open Option in
  get_pos name
  >>= (fun pos -> find_in_file (Pos.filename pos) name >>| get_mode)

let get_fun_mode =
  make_mode_getter
    ~get_pos:get_fun_pos
    ~find_in_file:Ast_provider.find_fun_in_file
    ~get_mode:(fun fun_ -> fun_.Aast.f_mode)

let get_class_mode =
  make_mode_getter
    ~get_pos:get_class_pos
    ~find_in_file:Ast_provider.find_class_in_file
    ~get_mode:(fun class_ -> class_.Aast.c_mode)

let get_typedef_mode =
  make_mode_getter
    ~get_pos:get_typedef_pos
    ~find_in_file:Ast_provider.find_typedef_in_file
    ~get_mode:(fun typedef -> typedef.Aast.t_mode)

let get_gconst_mode =
  make_mode_getter
    ~get_pos:get_gconst_pos
    ~find_in_file:Ast_provider.find_gconst_in_file
    ~get_mode:(fun gconst -> gconst.Aast.cst_mode)

let get_class_or_typedef_mode name =
  Option.first_some (get_class_mode name) (get_typedef_mode name)

let get_dep_mode dep =
  let open Typing_deps.Dep in
  match dep with
  | Fun name
  | FunName name ->
    get_fun_mode name
  | Class name
  | Const (name, _)
  | Method (name, _)
  | SMethod (name, _)
  | Prop (name, _)
  | SProp (name, _)
  | Cstr name
  | AllMembers name
  | Extends name ->
    get_class_or_typedef_mode name
  | GConst name
  | GConstName name ->
    get_gconst_mode name
  | RecordDef _ -> records_not_supported ()

let is_strict_dep dep = get_dep_mode dep = Some FileInfo.Mstrict

let is_strict_fun name = is_strict_dep (Typing_deps.Dep.Fun name)

let is_strict_class name = is_strict_dep (Typing_deps.Dep.Class name)

let is_hhi file_name =
  String_utils.string_ends_with (Relative_path.suffix file_name) ".hhi"

let is_builtin_dep dep =
  let open Typing_deps.Dep in
  match dep with
  | GConst c
  | GConstName c ->
    Naming_special_names.PseudoConsts.is_pseudo_const c
  | Fun _
  | FunName _
  | Class _
  | Const _
  | Method _
  | SMethod _
  | Prop _
  | SProp _
  | Cstr _
  | AllMembers _
  | Extends _ ->
    let msg = Typing_deps.Dep.to_string dep in
    let pos = value_or_not_found msg (get_dep_pos dep) in
    is_hhi (Pos.filename pos)
  | RecordDef _ -> records_not_supported ()

let is_relevant_dependency (target : target) (dep : Typing_deps.Dep.variant) =
  match target with
  | Function f ->
    dep = Typing_deps.Dep.Fun f || dep = Typing_deps.Dep.FunName f
  (* We have to collect dependencies of the entire class because dependency collection is
     coarse-grained: if cls's member depends on D, we get a dependency edge cls --> D,
     not (cls, member) --> D *)
  | Method (cls, _) -> get_class_name dep = Some cls

let get_filename target =
  let pos =
    match target with
    | Function func ->
      let f = value_exn NotFound @@ Decl_provider.get_fun func in
      Reason.to_pos (fst f.fe_type)
    | Method (cls, _) ->
      let cls = value_exn NotFound @@ Decl_provider.get_class cls in
      Decl_provider.Class.pos cls
  in
  Pos.filename pos

let extract_body target =
  let filename = get_filename target in
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

let format text =
  try Libhackfmt.format_tree (tree_from_string text)
  with Hackfmt_error.InvalidSyntax -> text

let name_from_type (_, ty) =
  match ty with
  | Tapply ((_, name), _) -> name
  | _ -> raise Unsupported

let strip_ns obj_name =
  match String.rsplit2 obj_name '\\' with
  | Some (_, name) -> name
  | None -> obj_name

let list_items items = String.concat items ~sep:", "

let tparam_name (tp : Typing_defs.decl_tparam) = snd tp.tp_name

let function_make_default = "extract_standalone_make_default"

let call_make_default = Printf.sprintf "\\%s()" function_make_default

let print_fun_args tcopt fun_type =
  let with_default arg_idx =
    match fun_type.ft_arity with
    | Fstandard (min, _) -> arg_idx >= min
    | Fvariadic _
    | Fellipsis _ ->
      false
  in
  let print_arg ?is_variadic:(var = false) idx arg =
    let name =
      match arg.fp_name with
      | Some n -> n
      | None -> ""
    in
    let inout =
      if arg.fp_kind = FPinout then
        "inout "
      else
        ""
    in
    let typ =
      match snd arg.fp_type.et_type with
      | Typing_defs.Tany _ -> ""
      | _ ->
        Printf.sprintf "%s "
        @@ Typing_print.full_decl tcopt arg.fp_type.et_type
    in
    let default =
      if with_default idx then
        Printf.sprintf " = %s" call_make_default
      else
        ""
    in
    if var then
      Printf.sprintf "%s...%s" typ name
    else
      Printf.sprintf "%s%s%s%s" inout typ name default
  in
  let args =
    String.concat ~sep:", " @@ List.mapi fun_type.ft_params print_arg
  in
  let variadic =
    match fun_type.ft_arity with
    (* variadic argument comes last *)
    | Fvariadic (arity, arg) -> print_arg ~is_variadic:true arity arg
    | Fstandard _
    | Fellipsis _ ->
      ""
  in
  if String.is_empty args then
    variadic
  else
    Printf.sprintf "%s, %s" args variadic

let get_function_declaration tcopt fun_name fun_type =
  let tparams =
    match fun_type.ft_tparams with
    | ([], _) -> ""
    | (tparams, _) ->
      Printf.sprintf "<%s>" @@ list_items @@ List.map tparams tparam_name
  in
  let args = print_fun_args tcopt fun_type in
  let rtype =
    match snd fun_type.ft_ret.et_type with
    | Typing_defs.Tany _ -> ""
    | _ ->
      Printf.sprintf ": %s"
      @@ Typing_print.full_decl tcopt fun_type.ft_ret.et_type
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

let get_enum_value enum_name =
  let enum =
    value_exn UnexpectedDependency @@ Decl_provider.get_class enum_name
  in
  if Decl_provider.Class.kind enum <> Ast_defs.Cenum then
    raise Unsupported
  else
    Decl_provider.Class.consts enum
    |> Sequence.map ~f:fst
    |> Sequence.find ~f:(fun name -> name <> "class")
    |> value_exn UnexpectedDependency

let get_init_from_type tcopt ty =
  Typing_defs.(
    let rec get_ (_, ty) =
      match ty with
      | Tprim prim -> get_init_for_prim prim
      | Toption _ -> "null"
      | Tarray _ -> "[]"
      | Tdarray _ -> "darray[]"
      | Tvarray _ -> "varray[]"
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
          | [cls] ->
            Printf.sprintf "%s::class" (Typing_print.full_decl tcopt cls)
          | _ -> raise Unsupported)
        | _ ->
          (match Decl_provider.get_class name with
          | Some _ -> Printf.sprintf "%s::%s" name (get_enum_value name)
          | None ->
            let typedef =
              value_exn UnexpectedDependency @@ Decl_provider.get_typedef name
            in
            get_ typedef.td_type))
      | Tshape (kind, fields) ->
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
        let open_shape =
          if kind = Open_shape then
            "..."
          else
            ""
        in
        Printf.sprintf
          "shape(%s%s)"
          (list_items @@ Nast.ShapeMap.fold print_shape_field fields [])
          open_shape
      | _ -> raise Unsupported
    in
    get_ ty)

let get_const_declaration tcopt ?abstract:(is_abstract = false) ty name =
  let abstract =
    if is_abstract then
      "abstract "
    else
      ""
  in
  let typ = Typing_print.full_decl tcopt ty in
  let init = get_init_from_type tcopt ty in
  Printf.sprintf "%sconst %s %s = %s;" abstract typ name init

let get_global_object_declaration tcopt obj =
  Typing_deps.Dep.(
    let description = to_string obj in
    match obj with
    | Fun f
    | FunName f ->
      let fun_type =
        value_or_not_found description @@ Decl_provider.get_fun f
      in
      begin
        match fun_type with
        | { fe_type = (_, Tfun fun_type); _ } ->
          let declaration = get_function_declaration tcopt f fun_type in
          declaration ^ "{throw new \\Exception();}"
        | _ -> failwith "Expected function type"
      end
    | GConst c
    | GConstName c ->
      let (const_type, _) =
        value_or_not_found description @@ Decl_provider.get_gconst c
      in
      get_const_declaration tcopt const_type (strip_ns c)
    (* No other global declarations *)
    | _ -> raise UnexpectedDependency)

type ancestors = {
  extends: decl_ty list;
  implements: decl_ty list;
  uses: decl_ty list;
  req_extends: decl_ty list;
  req_implements: decl_ty list;
}

let get_direct_ancestors cls =
  Decl_provider.(
    let set_of_sequence seq =
      Sequence.fold seq ~f:(fun set x -> SSet.add x set) ~init:SSet.empty
    in
    let get_direct (get_ancestors : Class.t -> string Sequence.t) =
      let ancestors = get_ancestors cls in
      let ancestors_ancestors =
        Sequence.concat_map ancestors ~f:(fun ancestor_name ->
            Option.value_map
              (get_class ancestor_name)
              ~default:Sequence.empty
              ~f:get_ancestors)
        |> set_of_sequence
      in
      Sequence.filter ancestors ~f:(fun x ->
          not (SSet.mem x ancestors_ancestors))
    in
    let direct_ancestors = get_direct Class.all_ancestor_names in
    let direct_reqs = get_direct Class.all_ancestor_req_names in
    let cls_kind = Class.kind cls in
    let filter_by_kind seq kind_condition =
      Sequence.to_list
      @@ Sequence.filter seq ~f:(fun n ->
             kind_condition cls_kind (Class.kind (get_class_exn n)))
    in
    let get_ancestor_types names =
      List.map names ~f:(fun n ->
          value_or_not_found n @@ Class.get_ancestor cls n)
    in
    let get_req_types names =
      let names_set = SSet.of_list names in
      Sequence.to_list
      @@ Sequence.filter
           (Sequence.map (Class.all_ancestor_reqs cls) ~f:snd)
           ~f:(fun ty -> SSet.mem (name_from_type ty) names_set)
    in
    Ast_defs.
      {
        extends =
          get_ancestor_types
          @@ filter_by_kind direct_ancestors (fun kderived kbase ->
                 (kderived = Cinterface && kbase = Cinterface)
                 || kbase = Cabstract
                 || kbase = Cnormal);
        implements =
          get_ancestor_types
          @@ filter_by_kind direct_ancestors (fun kderived kbase ->
                 kderived <> Cinterface && kbase = Cinterface);
        uses =
          get_ancestor_types
          @@ filter_by_kind direct_ancestors (fun _ kbase -> kbase = Ctrait);
        req_extends =
          get_req_types
          @@ filter_by_kind direct_reqs (fun _ kbase ->
                 kbase = Cabstract || kbase = Cnormal);
        req_implements =
          get_req_types
          @@ filter_by_kind direct_reqs (fun _ kbase -> kbase = Cinterface);
      })

let get_class_declaration tcopt (cls : Decl_provider.class_decl) =
  Decl_provider.(
    let kind =
      match Class.kind cls with
      | Ast_defs.Cabstract -> "abstract class"
      | Ast_defs.Cnormal -> "class"
      | Ast_defs.Cinterface -> "interface"
      | Ast_defs.Ctrait -> "trait"
      | Ast_defs.Cenum -> "enum"
    in
    let name = strip_ns @@ Class.name cls in
    let tparams =
      if List.is_empty @@ Class.tparams cls then
        ""
      else
        Printf.sprintf
          "<%s>"
          (list_items @@ List.map (Class.tparams cls) tparam_name)
    in
    let { extends; implements; _ } = get_direct_ancestors cls in
    let prefix_if_nonempty prefix s =
      if s = "" then
        ""
      else
        prefix ^ s
    in
    let extends =
      prefix_if_nonempty " extends "
      @@ list_items
      @@ List.map ~f:(Typing_print.full_decl tcopt) extends
    in
    let implements =
      prefix_if_nonempty " implements "
      @@ list_items
      @@ List.map ~f:(Typing_print.full_decl tcopt) implements
    in
    (* TODO: traits, records *)
    Printf.sprintf "%s %s%s%s%s" kind name tparams extends implements)

let get_method_declaration
    tcopt
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
    "%s%s %s%s"
    abstract
    visibility
    static
    (get_function_declaration tcopt method_name method_type)

let get_property_declaration
    tcopt (prop : Typing_defs.class_elt) ~is_static name =
  let visibility = Typing_utils.string_of_visibility prop.ce_visibility in
  let static =
    if is_static then
      "static "
    else
      ""
  in
  let ty = Lazy.force prop.ce_type in
  let prop_type = Typing_print.full_decl tcopt ty in
  let init =
    if is_static && not prop.ce_abstract then
      Printf.sprintf " = %s" (get_init_from_type tcopt ty)
    else
      ""
  in
  Printf.sprintf "%s %s%s $%s%s;" visibility static prop_type name init

let get_typeconst_declaration tcopt tconst name =
  let abstract =
    match tconst.ttc_abstract with
    | Typing_defs.TCAbstract _ -> "abstract "
    | TCPartiallyAbstract
    | TCConcrete ->
      ""
  in
  let typ =
    match tconst.ttc_type with
    | Some t -> Printf.sprintf " = %s" (Typing_print.full_decl tcopt t)
    | None -> ""
  in
  let constr =
    match tconst.ttc_constraint with
    | Some t -> Printf.sprintf " as %s" (Typing_print.full_decl tcopt t)
    | None -> ""
  in
  Printf.sprintf "%s const type %s%s%s;" abstract name constr typ

let get_class_elt_declaration
    tcopt
    (cls : Decl_provider.class_decl)
    (class_elt : Typing_deps.Dep.variant) =
  Typing_deps.Dep.(
    Decl_provider.(
      let from_interface = Class.kind cls = Ast_defs.Cinterface in
      let description = to_string class_elt in
      match class_elt with
      | Const (_, const_name) ->
        if Class.has_typeconst cls const_name then
          let tconst =
            value_or_not_found description
            @@ Class.get_typeconst cls const_name
          in
          get_typeconst_declaration tcopt tconst const_name
        else
          let cons =
            value_or_not_found description @@ Class.get_const cls const_name
          in
          get_const_declaration
            tcopt
            ~abstract:cons.cc_abstract
            cons.cc_type
            const_name
      | Method (_, method_name) ->
        let m =
          value_or_not_found description @@ Class.get_method cls method_name
        in
        let decl =
          get_method_declaration
            tcopt
            m
            ~from_interface
            ~is_static:false
            method_name
        in
        let body =
          if m.ce_abstract then
            ";"
          else
            "{throw new Exception();}"
        in
        decl ^ body
      | SMethod (_, smethod_name) ->
        let m =
          value_or_not_found description @@ Class.get_smethod cls smethod_name
        in
        let decl =
          get_method_declaration
            tcopt
            m
            ~from_interface
            ~is_static:true
            smethod_name
        in
        let body =
          if m.ce_abstract then
            ";"
          else
            "{throw new Exception();}"
        in
        decl ^ body
      | Prop (_, prop_name) ->
        let p =
          value_or_not_found description @@ Class.get_prop cls prop_name
        in
        get_property_declaration tcopt p ~is_static:false prop_name
      | SProp (_, sprop_name) ->
        let sp =
          value_or_not_found description @@ Class.get_sprop cls sprop_name
        in
        get_property_declaration
          tcopt
          sp
          ~is_static:true
          (String.lstrip ~drop:(fun c -> c = '$') sprop_name)
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

let construct_enum tcopt enum fields =
  let enum_name = Decl_provider.Class.name enum in
  let enum_type =
    value_exn UnexpectedDependency @@ Decl_provider.Class.enum_type enum
  in
  let string_enum_const = function
    | Typing_deps.Dep.Const (_, name) ->
      (* Say we have an
         enum MyEnum {
           FIRST = 1;
         }
         To generate an initializer for FIRST, we should pass MyEnum's base type (int),
         and not FIRST's type (MyEnum) *)
      Printf.sprintf
        "%s = %s;"
        name
        (get_init_from_type tcopt enum_type.te_base)
    | _ -> raise UnexpectedDependency
  in
  let base = Typing_print.full_decl tcopt enum_type.te_base in
  let cons =
    match enum_type.te_constraint with
    | Some c -> " as " ^ Typing_print.full_decl tcopt c
    | None -> ""
  in
  let enum_decl =
    Printf.sprintf "enum %s: %s%s" (strip_ns enum_name) base cons
  in
  let constants = List.map fields ~f:string_enum_const in
  Printf.sprintf "%s {%s}" enum_decl (String.concat ~sep:"\n" constants)

let get_constructor_declaration tcopt cls prop_names =
  let (cstr, _) = Decl_provider.Class.construct cls in
  match cstr with
  | None ->
    if List.is_empty prop_names then
      None
    else
      Some "public function __construct() {throw new \\Exception();}"
  | Some cstr ->
    let decl =
      get_method_declaration tcopt cstr ~is_static:false "__construct"
    in
    Some (decl ^ " {throw new \\Exception();}")

let get_class_body tcopt cls meth class_elts =
  let { uses; req_extends; req_implements; _ } = get_direct_ancestors cls in
  let uses =
    List.map uses ~f:(fun s ->
        Printf.sprintf "use %s;" (Typing_print.full_decl tcopt s))
  in
  let req_extends =
    List.map req_extends (fun s ->
        Printf.sprintf "require extends %s;" (Typing_print.full_decl tcopt s))
  in
  let req_implements =
    List.map req_implements (fun s ->
        Printf.sprintf
          "require implements %s;"
          (Typing_print.full_decl tcopt s))
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
        | Dep.Cstr _ -> get_constructor_declaration tcopt cls prop_names
        | Dep.Const (_, "class") -> None
        | class_elt -> Some (get_class_elt_declaration tcopt cls class_elt))
  in
  (* If we are extracting a method of this class, we should declare it
     here, with stubs of other class elements. *)
  let extracted_method =
    match meth with
    | Some (Method _ as m) -> [extract_body m]
    | _ -> []
  in
  String.concat
    ~sep:"\n"
    (req_extends @ req_implements @ uses @ body @ extracted_method)

let construct_class tcopt cls ?(full_method = None) fields =
  (* Enum declaration have a very different format: for example, no 'const' keyword
     for values, which are actually just class constants *)
  if Decl_provider.Class.kind cls = Ast_defs.Cenum then
    construct_enum tcopt cls fields
  else
    let decl = get_class_declaration tcopt cls in
    let body = get_class_body tcopt cls full_method fields in
    Printf.sprintf "%s {%s}" decl body

let construct_typedef tcopt t =
  let not_found_msg = Printf.sprintf "typedef %s" t in
  let td = value_or_not_found not_found_msg @@ Decl_provider.get_typedef t in
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
  Printf.sprintf
    "%s %s%s = %s;"
    typ
    (strip_ns t)
    tparams
    (Typing_print.full_decl tcopt td.td_type)

let construct_type_declaration tcopt t ?(full_method = None) fields =
  match Decl_provider.get_class t with
  | Some cls -> construct_class tcopt cls ~full_method fields
  | None -> construct_typedef tcopt t

let rec do_add_dep deps dep =
  if
    dep <> Typing_deps.Dep.Class SN.Typehints.wildcard
    && (not (HashSet.mem deps dep))
    && not (is_builtin_dep dep)
  then (
    HashSet.add deps dep;
    add_signature_dependencies deps dep;
    Option.iter (get_class_name dep) ~f:(fun name ->
        do_add_dep deps (Typing_deps.Dep.Class name))
  )

and add_dep deps ?cls:(this_cls = None) ty : unit =
  let visitor =
    object (this)
      inherit [unit] Type_visitor.decl_type_visitor

      method! on_tapply _ _ (_, name) tyl =
        let dep = Typing_deps.Dep.Class name in
        do_add_dep deps dep;

        (* If we have a constant of a generic type, it can only be an
           array type, e.g., vec<A>, for which don't need values of A
           to generate an initializer. *)
        List.iter tyl ~f:(add_dep deps ~cls:this_cls);

        (* When adding a dependency on an enum, also add dependencies
           on all its values.  We need all the values and not only the
           ones used by the extracted code.  See the \with_switch test
           case. *)
        match Decl_provider.get_class name with
        | Some enum when Decl_provider.Class.kind enum = Ast_defs.Cenum ->
          Decl_provider.Class.consts enum
          |> Sequence.iter ~f:(fun (const_name, _) ->
                 if const_name <> "class" then
                   do_add_dep deps (Typing_deps.Dep.Const (name, const_name)))
        | _ -> ()

      method! on_taccess _ _ ((_, cls_type), tconsts) =
        let class_name =
          match cls_type with
          | Tapply ((_, name), _) -> name
          | Tthis -> Option.value_exn this_cls
          | _ -> raise UnexpectedDependency
        in
        let cls = get_class_exn class_name in
        match tconsts with
        | [] -> raise UnexpectedDependency
        (* Unfold Class::TConst1::TConst2[::...]: get TConst1 in Class, get its type T,
           continue adding dependencies of T::TConst2[::...] *)
        | (_, tconst) :: tconsts ->
          do_add_dep deps (Typing_deps.Dep.Const (class_name, tconst));
          (match Decl_provider.Class.get_typeconst cls tconst with
          | Some typeconst ->
            Option.fold ~f:this#on_type ~init:() typeconst.ttc_type;
            if not (List.is_empty tconsts) then (
              match typeconst.ttc_type with
              | Some tc_type ->
                (* What 'this' refers to inside of T? *)
                let tc_this =
                  match snd tc_type with
                  | Tapply ((_, name), _) -> Some name
                  | _ -> None
                in
                let taccess = Typing_defs.Taccess (tc_type, tconsts) in
                add_dep ~cls:tc_this deps (Typing_reason.Rnone, taccess)
              | None -> ()
            )
          | None -> ())
    end
  in
  visitor#on_type () ty

and add_signature_dependencies deps obj =
  let open Typing_deps.Dep in
  let open Decl_provider in
  let description = to_string obj in
  match get_class_name obj with
  | Some cls_name ->
    (match get_class cls_name with
    | None ->
      let td = value_or_not_found description @@ get_typedef cls_name in
      add_dep deps td.td_type
    | Some cls ->
      let add_dep = add_dep deps ~cls:(Some cls_name) in
      (match obj with
      | Prop (_, name) ->
        let p = value_or_not_found description @@ Class.get_prop cls name in
        add_dep @@ Lazy.force p.ce_type;

        (* We need to initialize properties in the constructor, add a dependency on it *)
        HashSet.add deps (Cstr cls_name)
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
            do_add_dep deps (Method (cls_name, name))
          | None -> raise (DependencyNotFound description)))
      | Const (_, name) ->
        (match Class.get_typeconst cls name with
        | Some tc ->
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
        let add_implementations interface_name =
          let interf = get_class_exn interface_name in
          if
            is_builtin_dep (Class interface_name)
            && Class.kind interf = Ast_defs.Cinterface
          then (
            let add_smethod_impl (method_name, _) =
              Class.get_smethod cls method_name
              |> Option.iter ~f:(fun elt ->
                     if elt.ce_origin = cls_name then
                       do_add_dep deps (SMethod (cls_name, method_name)))
            in
            let add_method_impl (method_name, _) =
              Class.get_method cls method_name
              |> Option.iter ~f:(fun elt ->
                     if elt.ce_origin = cls_name then
                       do_add_dep deps (Method (cls_name, method_name)))
            in
            Sequence.iter (Class.methods interf) ~f:add_method_impl;
            Sequence.iter (Class.smethods interf) ~f:add_smethod_impl
          )
        in
        Sequence.iter (Class.all_ancestor_names cls) add_implementations
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
      let func = value_or_not_found description @@ get_fun f in
      add_dep deps @@ func.fe_type
    | GConst c
    | GConstName c ->
      let (ty, _) = value_or_not_found description @@ get_gconst c in
      add_dep deps ty
    | _ -> raise UnexpectedDependency)

let get_dependency_origin cls (dep : Typing_deps.Dep.variant) =
  Decl_provider.(
    Typing_deps.Dep.(
      let description = to_string dep in
      let cls = value_or_not_found description @@ get_class cls in
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
        let sm =
          value_or_not_found description @@ Class.get_smethod cls name
        in
        sm.ce_origin
      | Const (_, name) ->
        let c = value_or_not_found description @@ Class.get_const cls name in
        c.cc_origin
      | Cstr cls -> cls
      | _ -> raise UnexpectedDependency))

let get_dependencies tcopt target =
  let dependencies = HashSet.create 0 in
  let add_dependency root obj =
    if is_relevant_dependency target root then do_add_dep dependencies obj
  in
  Typing_deps.add_dependency_callback "add_dependency" add_dependency;
  let filename = get_filename target in
  (* Collect dependencies through side effects of typechecking and remove
   * the target function/method from the set of dependencies to avoid
   * declaring it twice.
   *)
  let () =
    Typing_deps.Dep.(
      match target with
      | Function func ->
        let (_ : (Tast.def * Typing_env_types.global_tvenv) option) =
          Typing_check_service.type_fun tcopt filename func
        in
        HashSet.remove dependencies (Fun func);
        HashSet.remove dependencies (FunName func)
      | Method (cls, m) ->
        let (_ : (Tast.def * Typing_env_types.global_tvenv list) option) =
          Typing_check_service.type_class tcopt filename cls
        in
        HashSet.remove dependencies (Method (cls, m));
        HashSet.remove dependencies (SMethod (cls, m)))
  in
  HashSet.fold List.cons dependencies []

let group_class_dependencies_by_class dependencies =
  List.fold_left
    dependencies
    ~f:(fun acc obj ->
      Typing_deps.Dep.(
        match obj with
        | Class cls ->
          if SMap.has_key cls acc then
            acc
          else
            SMap.add cls [] acc
        | AllMembers _ -> acc
        | Extends _ -> acc
        | _ ->
          let cls = value_exn UnexpectedDependency (get_class_name obj) in
          let origin = get_dependency_origin cls obj in
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
      Option.value (SMap.get name declarations) ~default:[] @ acc
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
      "function %s(): nothing {throw new Exception();}"
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

let get_declarations tcopt target class_dependencies global_dependencies =
  let (strict_class_dependencies, partial_class_dependencies) =
    SMap.partition (fun cls _ -> is_strict_class cls) class_dependencies
  in
  let (strict_global_dependencies, partial_global_dependencies) =
    List.partition_tf global_dependencies ~f:is_strict_dep
  in
  let add_declaration declarations name declaration =
    SMap.add name [declaration] declarations ~combine:(fun x y -> y @ x)
  in
  let add_global_declaration declarations dep =
    add_declaration
      declarations
      (global_dep_name dep)
      (get_global_object_declaration tcopt dep)
  in
  let add_class_declaration cls fields declarations =
    let full_method =
      match target with
      | Function _ -> None
      | Method (c, _) ->
        if c = cls then
          Some target
        else
          None
    in
    add_declaration
      declarations
      cls
      (construct_type_declaration tcopt cls ~full_method fields)
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
    let decl = extract_body target in
    if is_strict_fun name then
      (add_declaration strict_declarations name decl, partial_declarations)
    else
      (strict_declarations, add_declaration partial_declarations name decl)
  | Method _ -> (strict_declarations, partial_declarations)

let go tcopt target =
  try
    let dependencies = get_dependencies tcopt target in
    let (class_dependencies, global_dependencies) =
      List.partition_tf dependencies ~f:(fun dep ->
          Option.is_some (get_class_name dep))
    in
    let (strict_declarations, partial_declarations) =
      get_declarations
        tcopt
        target
        (group_class_dependencies_by_class class_dependencies)
        global_dependencies
    in
    get_code strict_declarations partial_declarations
  with
  | NotFound -> "Not found!"
  | InvalidInput ->
    "Unrecognized input. "
    ^ "Expected: fully qualified function name or [fully qualified class name]::[method_name]"
  | DependencyNotFound d -> Printf.sprintf "Dependency not found: %s" d
  | Unsupported
  | UnexpectedDependency ->
    Printexc.get_backtrace ()

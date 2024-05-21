(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SO = SymbolOccurrence
open Hh_prelude
open Option.Monad_infix

type kind =
  | Function
  | Class
  | Method
  | Property
  | ClassConst
  | GlobalConst
  | Enum
  | Interface
  | Trait
  | Typeconst
  | Typedef
  | Module
[@@deriving show]

let kind_to_string = function
  | Function -> "function"
  | Class -> "class"
  | Method -> "method"
  | Property -> "property"
  | ClassConst -> "class constant"
  | GlobalConst -> "const"
  | Enum -> "enum"
  | Interface -> "interface"
  | Trait -> "trait"
  | Typeconst -> "typeconst"
  | Typedef -> "typedef"
  | Module -> "module"

type t = {
  kind: kind;
  name: string;
  full_name: string;
}
[@@deriving show]

let member_name ~class_name ~name = class_name ^ "::" ^ name

let create_property class_name name =
  let kind = Property in
  let full_name = member_name ~class_name ~name in
  { kind; name; full_name }

let create_class_const class_name name =
  let kind = ClassConst in
  let full_name = member_name ~class_name ~name in
  { kind; name; full_name }

let create_class_typeconst class_name name =
  let kind = Typeconst in
  let full_name = member_name ~class_name ~name in
  { kind; name; full_name }

let create_method class_name name =
  let kind = Method in
  let full_name = member_name ~class_name ~name in
  { kind; name; full_name }

let create_class class_name kind =
  let kind =
    match kind with
    | Ast_defs.Cinterface -> Interface
    | Ast_defs.Ctrait -> Trait
    | Ast_defs.Cenum_class _
    | Ast_defs.Cenum ->
      Enum
    | Ast_defs.(Cclass _) -> Class
  in
  let name = class_name in
  let full_name = name in
  { kind; name; full_name }

let create_typedef name =
  let kind = Typedef in
  let full_name = name in
  { kind; name; full_name }

let create_fun name =
  let kind = Function in
  let full_name = name in
  { kind; name; full_name }

let create_gconst name =
  let kind = GlobalConst in
  let full_name = name in
  { kind; name; full_name }

let create_module_def name =
  let kind = Module in
  let full_name = name in
  { kind; name; full_name }

let create_class_typedef ctx class_ =
  Naming_provider.get_type_path_and_kind ctx class_ >>= fun (_fn, ct) ->
  match ct with
  | Naming_types.TClass ->
    Decl_provider.get_class ctx class_ |> Decl_entry.to_option
    >>= fun class_decl ->
    let kind = Folded_class.kind class_decl in
    Some (create_class class_ kind)
  | Naming_types.TTypedef -> Some (create_typedef class_)

let get_only_const class_ const_name =
  match Folded_class.get_typeconst class_ const_name with
  | None -> Folded_class.get_const class_ const_name
  | Some _ -> None

let resolve ctx SO.{ name; type_; _ } =
  let open Typing_defs in
  match type_ with
  | SO.Attribute (Some { SO.class_name; method_name; is_static }) ->
    let matching_method =
      Decl_provider.get_overridden_method
        ctx
        ~class_name
        ~method_name
        ~is_static
    in
    (match matching_method with
    | Decl_entry.Found meth -> Some (create_method meth.ce_origin method_name)
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      None)
  | SO.Attribute None
  | SO.Class _ ->
    create_class_typedef ctx name
  | SO.Method (SO.ClassName c_name, method_name) ->
    (* Classes on typing heap have all the methods from inheritance hierarchy
       * folded together, so we will correctly identify them even if method_name
       * is not defined directly in class c_name *)
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    if String.equal method_name Naming_special_names.Members.__construct then
      match fst (Folded_class.construct class_) with
      | Some m -> Some (create_method m.ce_origin method_name)
      | None ->
        let kind = Folded_class.kind class_ in
        Some (create_class c_name kind)
    else (
      match Folded_class.get_method class_ method_name with
      | Some m -> Some (create_method m.ce_origin method_name)
      | None ->
        Folded_class.get_smethod class_ method_name >>= fun m ->
        Some (create_method m.ce_origin method_name)
    )
  | SO.Property (SO.ClassName c_name, property_name)
  | SO.XhpLiteralAttr (c_name, property_name) ->
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    let property_name = IdentifySymbolService.clean_member_name property_name in
    begin
      match Folded_class.get_prop class_ property_name with
      | Some m -> Some (create_property m.ce_origin property_name)
      | None ->
        Folded_class.get_sprop class_ ("$" ^ property_name) >>= fun m ->
        Some (create_property m.ce_origin property_name)
    end
  | SO.Property (SO.UnknownClass, _) -> None
  | SO.ClassConst (SO.ClassName _, "class") -> None
  | SO.ClassConst (SO.ClassName c_name, const_name) ->
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    get_only_const class_ const_name >>= fun m ->
    Some (create_class_const m.cc_origin const_name)
  | SO.ClassConst (SO.UnknownClass, _) -> None
  | SO.EnumClassLabel (class_name, member_name) ->
    (* An enum class is a classish with class constants. *)
    Decl_provider.get_class ctx class_name |> Decl_entry.to_option
    >>= fun class_ ->
    get_only_const class_ member_name >>= fun m ->
    Some (create_class_const m.cc_origin member_name)
  | SO.Function ->
    Decl_provider.get_fun ctx name |> Decl_entry.to_option >>= fun _fun_decl ->
    Some (create_fun name)
  | SO.GConst -> Some (create_gconst name)
  | SO.Typeconst (c_name, typeconst_name) ->
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    Folded_class.get_typeconst class_ typeconst_name >>= fun m ->
    Some (create_class_typeconst m.ttc_origin typeconst_name)
  | SO.Module -> Some (create_module_def name)
  | SO.LocalVar -> None
  | SO.TypeVar -> None
  | SO.HhFixme -> None
  | SO.Method (SO.UnknownClass, _) -> None
  | SO.Keyword _ -> None
  | SO.PureFunctionContext -> None
  | SO.BuiltInType _ -> None
  | SO.BestEffortArgument _ -> None

let get_kind ctx class_ =
  Decl_provider.get_class ctx class_ |> Decl_entry.to_option
  >>= fun class_decl -> Some (Folded_class.kind class_decl)

let get_overridden_method_origin ctx ~class_name ~method_name ~is_static =
  Decl_provider.get_overridden_method ctx ~class_name ~method_name ~is_static
  |> Decl_entry.to_option
  >>= fun method_ ->
  Some method_.Typing_defs.ce_origin >>= fun origin ->
  get_kind ctx origin >>= fun kind ->
  Some (origin, Predicate.get_parent_kind kind)

let get_class_name full_name =
  match Str.split (Str.regexp "::") full_name with
  | con :: _mem -> Some con
  | _ -> None

let filename ctx { kind; name; full_name } =
  match kind with
  | Function -> Naming_provider.get_fun_path ctx name
  | Module -> Naming_provider.get_module_path ctx name
  | Class
  | Enum
  | Interface
  | Trait ->
    Naming_provider.get_class_path ctx name
  | Typedef -> Naming_provider.get_typedef_path ctx name
  | GlobalConst -> Naming_provider.get_const_path ctx name
  | Method
  | Property
  | ClassConst
  | Typeconst ->
    get_class_name full_name >>= Naming_provider.get_class_path ctx

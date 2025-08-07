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

(* We don't introduce new dependencies here,
   * so it's safe to skip dependency tracking *)
[@@@alert "-dependencies"]

type t =
  | Function of { name: string }
  | Method of {
      class_name: string;
      name: string;
    }
  | Property of {
      class_name: string;
      name: string;
    }
  | ClassConst of {
      class_name: string;
      name: string;
    }
  | GlobalConst of { name: string }
  | Class of {
      kind: Ast_defs.classish_kind;
      name: string;
    }
  | Typeconst of {
      class_name: string;
      name: string;
    }
  | Typedef of { name: string }
  | Module of { name: string }
[@@deriving show]

let create_class_typedef ctx class_ =
  Naming_provider.get_type_path_and_kind ctx class_ >>= fun (_fn, ct) ->
  match ct with
  | Naming_types.TClass ->
    Decl_provider.get_class ctx class_ |> Decl_entry.to_option
    >>= fun class_decl ->
    let kind = Folded_class.kind class_decl in
    Some (Class { kind; name = class_ })
  | Naming_types.TTypedef -> Some (Typedef { name = class_ })

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
    | Decl_entry.Found meth ->
      Some (Method { class_name = meth.ce_origin; name = method_name })
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
      | Some m -> Some (Method { class_name = m.ce_origin; name = method_name })
      | None ->
        let kind = Folded_class.kind class_ in
        Some (Class { kind; name = c_name })
    else
      (match Folded_class.get_method class_ method_name with
      | Some m -> Some m
      | None -> Folded_class.get_smethod class_ method_name)
      >>| fun m -> Method { class_name = m.ce_origin; name = method_name }
  | SO.Property (SO.ClassName c_name, property_name)
  | SO.XhpLiteralAttr (c_name, property_name) ->
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    let name = IdentifySymbolService.clean_member_name property_name in
    (match Folded_class.get_prop class_ property_name with
    | Some m -> Some m
    | None -> Folded_class.get_sprop class_ ("$" ^ property_name))
    >>| fun m -> Property { class_name = m.ce_origin; name }
  | SO.Property (SO.UnknownClass, _) -> None
  | SO.ClassConst (SO.ClassName _, "class") -> None
  | SO.ClassConst (SO.ClassName c_name, const_name) ->
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    get_only_const class_ const_name >>= fun m ->
    Some (ClassConst { class_name = m.cc_origin; name = const_name })
  | SO.ClassConst (SO.UnknownClass, _) -> None
  | SO.EnumClassLabel (class_name, name) ->
    (* An enum class is a classish with class constants. *)
    Decl_provider.get_class ctx class_name |> Decl_entry.to_option
    >>= fun class_ ->
    get_only_const class_ name >>= fun m ->
    Some (ClassConst { class_name = m.cc_origin; name })
  | SO.Function ->
    Decl_provider.get_fun ctx name |> Decl_entry.to_option >>= fun _fun_decl ->
    Some (Function { name })
  | SO.GConst -> Some (GlobalConst { name })
  | SO.Typeconst (c_name, typeconst_name) ->
    Decl_provider.get_class ctx c_name |> Decl_entry.to_option >>= fun class_ ->
    Folded_class.get_typeconst class_ typeconst_name >>= fun m ->
    Some (Typeconst { class_name = m.ttc_origin; name = typeconst_name })
  | SO.Module -> Some (Module { name })
  | SO.LocalVar -> None
  | SO.TypeVar -> None
  | SO.HhFixme -> None
  | SO.HhIgnore -> None
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

let filename ctx = function
  | Function { name } -> Naming_provider.get_fun_path ctx name
  | Module { name } -> Naming_provider.get_module_path ctx name
  | Class { name; _ } -> Naming_provider.get_class_path ctx name
  | Typedef { name } -> Naming_provider.get_typedef_path ctx name
  | GlobalConst { name } -> Naming_provider.get_const_path ctx name
  | Method { class_name; _ }
  | Property { class_name; _ }
  | ClassConst { class_name; _ }
  | Typeconst { class_name; _ } ->
    Naming_provider.get_class_path ctx class_name

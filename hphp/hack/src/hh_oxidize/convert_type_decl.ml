(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
open Asttypes
open Longident
open Parsetree
open Printf
open Utils
open Output
open State
open Convert_longident
open Rust_type

let is_by_ref () =
  match Configuration.mode () with
  | Configuration.ByRef -> true
  | Configuration.ByBox -> false

let stringify_attribute { attr_name; attr_payload; _ } =
  match (attr_name, attr_payload) with
  | ({ txt = "ocaml.doc" | "value"; _ }, _) -> None
  | ({ txt; _ }, PStr []) -> Some txt
  | ({ txt; _ }, PStr [structure_item]) ->
    let item =
      structure_item
      |> Format.asprintf "%a" Pprintast.structure_item
      |> String.strip ~drop:(function
             | ' '
             | '\t'
             | ';' ->
               true
             | _ -> false)
    in
    Some (txt ^ " " ^ item)
  | _ -> None

let add_default_attr_if_ocaml_yojson_drop_if attributes acc_attr_list =
  let contains_yojson_drop_if attr =
    match stringify_attribute attr with
    | None -> false
    | Some attr -> String.is_prefix attr ~prefix:"yojson_drop_if"
  in
  if List.exists attributes ~f:contains_yojson_drop_if then
    "default" :: acc_attr_list
  else
    acc_attr_list

let add_deserialize_with_arena tys acc_attr_list =
  let contains_ref = List.exists ~f:Rust_type.contains_ref tys in
  if contains_ref || (is_by_ref () && List.exists ~f:Rust_type.is_var tys) then
    (* deserialize a type contains any Cell causes a compilation error, see T90211775 *)
    let contains_cell =
      List.exists
        ~f:(fun t ->
          Rust_type.type_name_and_params t
          |> fst
          |> String.is_suffix ~suffix:"::Cell")
        tys
    in
    if contains_cell then
      "skip" :: acc_attr_list
    else
      let acc_attr_list =
        if contains_ref then
          "borrow" :: acc_attr_list
        else
          acc_attr_list
      in
      "deserialize_with = \"arena_deserializer::arena\"" :: acc_attr_list
  else
    acc_attr_list

let rust_de_field_attr (tys : Rust_type.t list) (attributes : attributes) :
    string =
  let serde_attr_list =
    []
    |> add_default_attr_if_ocaml_yojson_drop_if attributes
    |> add_deserialize_with_arena tys
  in
  if List.is_empty serde_attr_list then
    ""
  else
    sprintf "#[serde(%s)]" @@ String.concat ~sep:", " serde_attr_list

let default_implements () =
  match Configuration.mode () with
  | Configuration.ByRef -> [(Some "arena_trait", "TrivialDrop")]
  | Configuration.ByBox -> []

let implements_traits _name = default_implements ()

let default_derives () =
  (match Configuration.mode () with
  | Configuration.ByBox ->
    [(Some "ocamlrep", "FromOcamlRep"); (Some "serde", "Deserialize")]
  | Configuration.ByRef -> [(Some "ocamlrep", "FromOcamlRepIn")])
  @ [
      (None, "Clone");
      (None, "Debug");
      (None, "Eq");
      (None, "Hash");
      (None, "Ord");
      (None, "PartialEq");
      (None, "PartialOrd");
      (Some "no_pos_hash", "NoPosHash");
      (Some "eq_modulo_pos", "EqModuloPos");
      (Some "ocamlrep", "ToOcamlRep");
      (Some "serde", "Serialize");
      (Some "serde", "Deserialize");
    ]

let derive_copy ty = Convert_type.is_copy (Rust_type.rust_simple_type ty)

let derive_default (ty : label) =
  List.mem ["tast_hashes::ByNames"] ty ~equal:String.equal

let is_by_box () = not (is_by_ref ())

let additional_derives ty : (string option * string) list =
  let result = [] in
  let result =
    if derive_copy ty then
      (None, "Copy") :: (Some "ocamlrep", "FromOcamlRepIn") :: result
    else
      result
  in
  let result =
    if derive_default ty then
      (None, "Default") :: result
    else
      result
  in
  result

module DeriveSkipLists : sig
  val skip_derive : ty:string -> trait:string -> bool
end = struct
  let skip_list_for_ty ty =
    let is_by_ref = is_by_ref () in
    match ty with
    (* A custom implementation of Ord for Error_ matches the sorting behavior of
       errors in OCaml. *)
    | "user_error::UserError" -> ["Ord"; "PartialOrd"]
    (* GlobalOptions contains a couple floats, which only implement PartialEq
       and PartialOrd, and do not implement Hash. *)
    | "global_options::GlobalOptions" ->
      ["Eq"; "EqModuloPos"; "Hash"; "NoPosHash"; "Ord"]
    (* And GlobalOptions is used in Genv which is used in Env. We
     * don't care about comparison or hashing on environments *)
    | "typing_env_types::Env" ->
      ["Eq"; "EqModuloPos"; "Hash"; "NoPosHash"; "Ord"]
    | "typing_env_types::Genv" ->
      ["Eq"; "EqModuloPos"; "Hash"; "NoPosHash"; "Ord"]
    (* And GlobalOptions is used in SavedEnv. *)
    | "tast::SavedEnv" -> ["Eq"; "EqModuloPos"; "Hash"; "NoPosHash"; "Ord"]
    | "tast::ByNames" -> ["Eq"; "EqModuloPos"; "Hash"; "NoPosHash"; "Ord"]
    | "ast_defs::Id" -> ["Debug"]
    | "errors::Errors" when is_by_ref -> ["Debug"]
    | "typing_reason::T_" when is_by_ref -> ["Debug"]
    | "typing_defs_core::Ty" when is_by_ref ->
      ["Eq"; "PartialEq"; "Ord"; "PartialOrd"]
    | "typing_defs_core::Ty_" -> ["Debug"]
    | "typing_defs_core::ConstraintType" when is_by_ref ->
      ["Eq"; "PartialEq"; "Ord"; "PartialOrd"]
    | "typing_defs_core::TshapeFieldName" when is_by_ref -> ["Debug"]
    | _ -> []

  let skip_list_for_trait trait =
    match trait with
    | "EqModuloPos" ->
      [
        "scoured_comments::*";
        "pos_or_decl::*";
        "namespace_env::*";
        "file_info::NameType";
        "file_info::Pos";
        "file_info::Id";
        "file_info::FileInfo";
        "file_info::Names";
        "file_info::SavedNames";
        "file_info::Saved";
        "file_info::Diff";
        "aast_defs::*";
        "nast::*";
        "tast::*";
        "full_fidelity_parser_env::*";
        "lints_core::*";
        "typing_env_types::*";
        "typing_tyvar_occurrences::*";
        "typing_per_cont_env::*";
        "typing_inference_env::*";
        "typing_kinding_defs::*";
        "type_parameter_env::*";
        "typing_fake_members::*";
        "typing_defs_core::HasMember";
        "typing_defs_core::Destructure";
        "typing_defs_core::DestructureKind";
        "typing_defs_core::ConstraintType_";
        "typing_defs_core::ConstraintType";
        "typing_defs_core::InternalType";
      ]
    | _ -> []

  let is_in_ty_skip_list ~ty ~trait =
    List.mem (skip_list_for_ty ty) trait ~equal:String.equal

  let is_in_trait_skip_list ~ty ~trait =
    let path_ty = String.split ty ~on:':' in
    List.exists (skip_list_for_trait trait) ~f:(fun skip_ty ->
        (* if skip_ty is like "SomeTy" then treat it as unqualified
         * and skip if any type like "some_path::SomeTy" is in the
         * skip list. Otherwise, just compare the fully qualified types,
         * modulo "*". *)
        match String.split skip_ty ~on:':' with
        | [skip_ty] ->
          (match List.last path_ty with
          | None -> false
          | Some ty -> String.equal ty skip_ty)
        | path_skip_ty ->
          List.equal
            (fun node skip_node ->
              String.equal node skip_node || String.equal "*" skip_node)
            path_ty
            path_skip_ty)

  let skip_derive ~ty ~trait =
    is_in_ty_skip_list ~ty ~trait || is_in_trait_skip_list ~ty ~trait
end

let derived_traits ty =
  let ty = sprintf "%s::%s" (curr_module_name ()) ty in
  default_derives ()
  |> List.filter ~f:(fun (_, trait) ->
         not (DeriveSkipLists.skip_derive ~ty ~trait))
  |> List.append (additional_derives ty)

let denylisted_types () =
  (match Configuration.mode () with
  | Configuration.ByRef ->
    [
      ("typing_defs_core", "CanIndex");
      ("typing_defs_core", "CanTraverse");
      ("typing_defs_core", "ConstraintType_");
      ("typing_defs_core", "ConstraintType");
      ("typing_defs_core", "Destructure");
      ("typing_defs_core", "DestructureKind");
      ("typing_defs_core", "HasMember");
      ("typing_defs_core", "HasTypeMember");
      ("typing_defs_core", "InternalType");
      ("nast", "Defs");
    ]
  | Configuration.ByBox -> [])
  @ [
      ("aast_defs", "LocalIdMap");
      ("aast_defs", "ByteString");
      ("errors", "FinalizedError");
      ("errors", "Marker");
      ("errors", "MarkedMessage");
      ("errors", "PositionGroup");
      ("file_info", "Saved");
      ("typing_defs", "ExpandEnv");
      ("typing_defs", "PhaseTy");
      ("typing_defs", "WildcardAction");
      ("typing_reason", "DeclPhase");
      ("typing_reason", "LoclPhase");
    ]

(* HACK: ignore anything beginning with the "decl" or "locl" prefix, since the
   oxidized version of Ty does not have a phase. *)
let denylisted_type_prefixes =
  [
    ("typing_defs", "Decl");
    ("typing_defs_core", "Decl");
    ("typing_defs", "Locl");
    ("typing_defs_core", "Locl");
  ]

(* HACK: Typing_reason is usually aliased to Reason, so we have lots of
   instances of Reason.t. Since we usually convert an identifier like Reason.t
   to reason::Reason, the actual type needs to be renamed to the common alias.
   This looks nicer anyway. *)
let renamed_types = [(("typing_reason", "TypingReason"), "Reason")]

(* By default, when we see an alias to a tuple type, we will assume the alias
   adds some meaning, and generate a new tuple struct type named after the
   alias. In some cases, the alias adds no meaning and we should also use an
   alias in Rust. *)
let tuple_aliases =
  [
    ("ast_defs", "Pstring");
    ("ast_defs", "PositionedByteString");
    ("errors", "Message");
    ("typing_reason", "PosId");
  ]

let newtypes =
  [
    ("aast_defs", "Block");
    ("aast_defs", "FinallyBlock");
    ("aast_defs", "Program");
    ("aast_defs", "UserAttributes");
    ("file_info", "HashType");
  ]

(*
A list of (<module>, <ty1>) where ty1 is enum and all non-empty variant fields should
be wrapped by Box to keep ty1 size down.
*)
let box_variant () =
  (match Configuration.mode () with
  | Configuration.ByRef -> [("typing_defs_core", "Ty_")]
  | Configuration.ByBox -> [])
  @ [
      ("aast_defs", "Expr_");
      ("aast_defs", "Stmt_");
      ("aast_defs", "Def");
      ("aast_defs", "Pattern");
    ]

let equal_s2 = [%derive.eq: string * string]

let should_box_variant ty =
  List.mem (box_variant ()) (curr_module_name (), ty) ~equal:equal_s2

(* When should_box_variant returns true, we will switch to boxing the fields of
   each variant by default. Some fields are small enough not to need boxing,
   though, so we opt out of the boxing behavior for them here to avoid
   unnecessary indirections. The rule of thumb I'm using here is that the size
   should be two words or less (the size of a slice). *)
let unbox_field ty =
  let open String in
  let is_copy = Convert_type.is_copy ty in
  let ty = Rust_type.rust_type_to_string ty in
  ty = "String"
  || ty = "bstr::BString"
  || is_prefix ty ~prefix:"Vec<"
  || is_prefix ty ~prefix:"Block<"
  || is_prefix ty ~prefix:"&'a "
  || is_prefix ty ~prefix:"Option<&'a "
  || is_prefix ty ~prefix:"std::cell::Cell<&'a "
  || is_prefix ty ~prefix:"std::cell::RefCell<&'a "
  ||
  match Configuration.mode () with
  | Configuration.ByRef ->
    ty = "tany_sentinel::TanySentinel"
    || ty = "ident::Ident"
    || ty = "ConditionTypeName<'a>"
    || ty = "ConstraintType<'a>"
    || (is_prefix ty ~prefix:"Option<" && is_copy)
    || (is_prefix ty ~prefix:"std::cell::Cell<" && is_copy)
    || (is_prefix ty ~prefix:"std::cell::RefCell<" && is_copy)
    || Convert_type.is_primitive ty
  | Configuration.ByBox -> false

let add_rcoc = [("aast_defs", "Nsenv"); ("aast", "Nsenv")]

let should_add_rcoc ty =
  match Configuration.mode () with
  | Configuration.ByRef -> false
  | Configuration.ByBox ->
    List.mem add_rcoc (curr_module_name (), ty) ~equal:equal_s2

let denylisted ty_name =
  let ty = (curr_module_name (), ty_name) in
  List.mem (denylisted_types ()) ty ~equal:equal_s2
  || List.exists denylisted_type_prefixes ~f:(fun (mod_name, prefix) ->
         String.equal mod_name (curr_module_name ())
         && String.is_prefix ty_name ~prefix)

let rename ty_name =
  List.find renamed_types ~f:(fun (x, _) ->
      equal_s2 x (curr_module_name (), ty_name))
  |> Option.value_map ~f:snd ~default:ty_name

let should_use_alias_instead_of_tuple_struct ty_name =
  let equal = [%derive.eq: string * string] in
  List.mem tuple_aliases (curr_module_name (), ty_name) ~equal

let should_be_newtype ty_name =
  let equal = [%derive.eq: string * string] in
  List.mem newtypes (curr_module_name (), ty_name) ~equal

let doc_comment_of_attribute { attr_name; attr_payload; _ } =
  match (attr_name, attr_payload) with
  | ({ txt = "ocaml.doc"; _ }, PStr structure_items) ->
    List.find_map structure_items ~f:(fun structure_item ->
        match structure_item.pstr_desc with
        | Pstr_eval
            ({ pexp_desc = Pexp_constant (Pconst_string (doc, _, _)); _ }, _) ->
          Some doc
        | _ -> None)
  | _ -> None

let convert_doc_comment doc =
  doc
  |> String.strip ~drop:(function
         | '*'
         | ' '
         | '\n'
         | '\t' ->
           true
         | _ -> false)
  |> String.split ~on:'\n'
  |> List.fold
       ~init:(false, [])
       ~f:(fun (was_in_code_block, lines) original_line ->
         (* Remove leading whitespace *)
         let lstripped = String.lstrip original_line in
         let maybe_chop_prefix prefix s =
           String.chop_prefix s ~prefix |> Option.value ~default:s
         in
         (* Remove leading asterisk and one space after, if present *)
         let no_asterisk =
           lstripped |> maybe_chop_prefix "*" |> maybe_chop_prefix " "
         in
         let now_in_code_block =
           if String.is_prefix ~prefix:"```" (String.lstrip no_asterisk) then
             not was_in_code_block
           else
             was_in_code_block
         in
         let line =
           if
             now_in_code_block
             && was_in_code_block
             && String.equal lstripped no_asterisk
           then
             sprintf "///%s\n" original_line
           else
             sprintf "/// %s\n" no_asterisk
         in
         (now_in_code_block, line :: lines))
  |> (fun (_, l) -> List.rev l)
  |> String.concat

let doc_comment_of_attribute_list attrs =
  attrs
  |> List.find_map ~f:doc_comment_of_attribute
  |> Option.map ~f:convert_doc_comment
  |> Option.value ~default:""

let ocaml_attr attrs =
  attrs
  |> List.filter_map ~f:stringify_attribute
  |> List.map ~f:(fun attr ->
         if String.contains attr '"' then
           Printf.sprintf "#[rust_to_ocaml(attr = r#\"%s\"#)]\n" attr
         else
           Printf.sprintf "#[rust_to_ocaml(attr = \"%s\")]\n" attr)
  |> String.concat ~sep:""

let type_param (ct, _) = Convert_type.core_type ct

let type_params name params =
  let params = List.map ~f:type_param params in
  let lifetime =
    match Configuration.mode () with
    | Configuration.ByRef ->
      if Configuration.owned_type name then
        []
      else
        [Rust_type.lifetime "a"]
    | Configuration.ByBox -> []
  in
  (lifetime, params)

let record_label_declaration
    ?(pub = false) ?(prefix = "") (ld : label_declaration) : label =
  let doc = doc_comment_of_attribute_list ld.pld_attributes in
  let attr = ocaml_attr ld.pld_attributes in
  let pub =
    if pub then
      "pub "
    else
      ""
  in
  let name =
    ld.pld_name.txt |> String.chop_prefix_exn ~prefix |> convert_field_name
  in
  let ty = Convert_type.core_type ld.pld_type in
  sprintf
    "%s%s%s%s %s: %s,\n"
    doc
    (rust_de_field_attr [ty] ld.pld_attributes)
    attr
    pub
    name
    (rust_type_to_string ty)

let find_record_label_prefix labels =
  let prefix =
    labels |> List.map ~f:(fun ld -> ld.pld_name.txt) |> common_prefix_of_list
  in
  (* Only remove a common prefix up to the last underscore (if a record has
     fields x_bar and x_baz, we want to remove x_, not x_ba). *)
  let idx = ref (String.length prefix) in
  while !idx > 0 && Char.(prefix.[!idx - 1] <> '_') do
    idx := !idx - 1
  done;
  String.sub prefix ~pos:0 ~len:!idx

let record_prefix_attr prefix =
  if String.is_empty prefix then
    ""
  else
    sprintf "#[rust_to_ocaml(prefix = \"%s\")]\n" prefix

let declare_record_arguments ?(pub = false) ~prefix labels =
  labels
  |> map_and_concat ~f:(record_label_declaration ~pub ~prefix)
  |> sprintf "{\n%s}"

let declare_constructor_arguments ?(box_fields = false) types : Rust_type.t list
    =
  if not box_fields then
    if List.is_empty types then
      []
    else
      List.map ~f:Convert_type.core_type types
  else
    match types with
    | [] -> []
    | [ty] ->
      let ty = Convert_type.core_type ty in
      let ty =
        if unbox_field ty then
          ty
        else
          match Configuration.mode () with
          | Configuration.ByRef -> rust_ref (lifetime "a") ty
          | Configuration.ByBox -> rust_type "Box" [] [ty]
      in
      [ty]
    | _ ->
      (match Configuration.mode () with
      | Configuration.ByRef ->
        let tys = Convert_type.tuple ~seen_indirection:true types in
        [rust_ref (lifetime "a") tys]
      | Configuration.ByBox -> [rust_type "Box" [] [Convert_type.tuple types]])

let variant_constructor_value cd =
  (* If we see the [@value 42] attribute, assume it's for ppx_deriving enum,
     and that all the variants are zero-argument (i.e., assume this is a
     C-like enum and provide custom discriminant values). *)
  List.find_map cd.pcd_attributes ~f:(fun { attr_name; attr_payload; _ } ->
      match (attr_name, attr_payload) with
      | ( { txt = "value"; _ },
          PStr
            [
              {
                pstr_desc =
                  Pstr_eval
                    ( {
                        pexp_desc =
                          Pexp_constant (Pconst_integer (discriminant, None));
                        _;
                      },
                      _ );
                _;
              };
            ] ) ->
        Some discriminant
      | _ -> None)

let variant_constructor_declaration ?(box_fields = false) cd =
  let doc = doc_comment_of_attribute_list cd.pcd_attributes in
  let attr = ocaml_attr cd.pcd_attributes in
  let name = convert_type_name cd.pcd_name.txt in
  let name_attr =
    if String.equal name cd.pcd_name.txt then
      ""
    else
      sprintf "#[rust_to_ocaml(name = \"%s\")]\n" cd.pcd_name.txt
  in
  let value =
    variant_constructor_value cd
    |> Option.value_map ~f:(( ^ ) " = ") ~default:""
  in
  match cd.pcd_args with
  | Pcstr_tuple types ->
    let tys = declare_constructor_arguments ~box_fields types in
    sprintf
      "%s%s%s%s%s%s%s%s,\n"
      doc
      (rust_de_field_attr tys cd.pcd_attributes)
      attr
      name_attr
      (if box_fields && List.length types > 1 then
        "#[rust_to_ocaml(inline_tuple)]"
      else
        "")
      name
      (if List.is_empty tys then
        ""
      else
        map_and_concat ~sep:"," ~f:rust_type_to_string tys |> sprintf "(%s)")
      value
  | Pcstr_record labels ->
    let prefix = find_record_label_prefix labels in
    sprintf
      "%s%s%s%s%s%s%s,\n"
      doc
      attr
      (record_prefix_attr prefix)
      name_attr
      name
      (declare_record_arguments labels ~prefix)
      value

let ctor_arg_len (ctor_args : constructor_arguments) : int =
  match ctor_args with
  | Pcstr_tuple x -> List.length x
  | Pcstr_record x -> List.length x

(* When converting a variant type to a Rust enum, consider whether the enum will
   be "C-like" (i.e., a type where all variants take no arguments), and if so,
   what the maximum [@value] annotation was. *)
type enum_kind =
  | C_like of {
      max_value: int;
      num_variants: int;
    }
  | Sum_type of { num_variants: int }
  | Not_an_enum

let type_declaration ~mutual_rec name td =
  let tparam_list =
    match (td.ptype_params, td.ptype_name.txt) with
    (* HACK: eliminate tparam from `type _ ty_` and phase-parameterized types *)
    | ([({ ptyp_desc = Ptyp_any; _ }, _)], "ty_")
    | ([({ ptyp_desc = Ptyp_var "phase"; _ }, _)], _)
    | ([({ ptyp_desc = Ptyp_var "ty"; _ }, _)], _)
      when String.(
             curr_module_name () = "typing_defs_core"
             || curr_module_name () = "typing_defs") ->
      []
    | ([({ ptyp_desc = Ptyp_any; _ }, _)], "t_")
      when String.(curr_module_name () = "typing_reason") ->
      []
    | (tparams, _) -> tparams
  in
  let (lifetime, tparams) = type_params name tparam_list in
  let serde_attr =
    if List.is_empty lifetime || List.is_empty tparams then
      ""
    else
      let bounds =
        map_and_concat
          ~sep:", "
          ~f:(fun v ->
            sprintf
              "%s: 'de + arena_deserializer::DeserializeInArena<'de>"
              (Rust_type.rust_type_to_string v))
          tparams
      in
      sprintf "#[serde(bound(deserialize = \"%s\" ))]" bounds
  in
  let doc = doc_comment_of_attribute_list td.ptype_attributes in
  let attr = ocaml_attr td.ptype_attributes in
  let attr =
    if mutual_rec then
      "#[rust_to_ocaml(and)]\n" ^ attr
    else
      attr
  in
  let attrs_and_vis ?(additional_attrs = "") enum_kind ~force_derive_copy =
    if
      force_derive_copy
      && Configuration.is_known (Configuration.copy_type name) false
    then
      failwith
        (Printf.sprintf
           "Type %s::%s can implement Copy but is not specified in the copy_types file. Please add it."
           (curr_module_name ())
           name);
    let additional_derives =
      if force_derive_copy then
        [(None, "Copy")]
      else
        []
    in
    let derive_attr =
      let traits = derived_traits name @ additional_derives in
      let traits =
        match enum_kind with
        | C_like _ -> (Some "ocamlrep", "FromOcamlRep") :: traits
        | _ -> traits
      in
      let traits =
        if force_derive_copy then
          (Some "ocamlrep", "FromOcamlRepIn") :: traits
        else
          traits
      in
      traits
      |> List.dedup_and_sort ~compare:(fun (_, t1) (_, t2) ->
             String.compare t1 t2)
      |> List.map ~f:(fun (m, trait) ->
             Option.iter m ~f:(fun m -> add_extern_use (m ^ "::" ^ trait));
             trait)
      |> String.concat ~sep:", "
      |> sprintf "#[derive(%s)]"
    in
    let repr =
      match enum_kind with
      | C_like { max_value; num_variants }
        when max num_variants (max_value + 1) <= 256 ->
        "\n#[repr(u8)]"
      | Sum_type { num_variants } when num_variants <= 256 -> "\n#[repr(C, u8)]"
      | _ -> "\n#[repr(C)]"
    in
    doc ^ derive_attr ^ serde_attr ^ attr ^ additional_attrs ^ repr ^ "\npub"
  in
  let deserialize_in_arena_macro ~force_derive_copy =
    if is_by_ref () || force_derive_copy || String.equal name "EmitId" then
      let lts = List.map lifetime ~f:(fun _ -> Rust_type.lifetime "arena") in
      sprintf
        "arena_deserializer::impl_deserialize_in_arena!(%s%s);\n"
        name
        (type_params_to_string lts tparams)
    else
      ""
  in
  let implements ~force_derive_copy =
    let traits = implements_traits name in
    let traits =
      if force_derive_copy then
        (Some "arena_trait", "TrivialDrop") :: traits
      else
        traits
    in
    traits
    |> List.dedup_and_sort ~compare:(fun (_, t1) (_, t2) ->
           String.compare t1 t2)
    |> List.map ~f:(fun (m, trait) ->
           Option.iter m ~f:(fun m -> add_extern_use (m ^ "::" ^ trait));
           trait)
    |> List.map ~f:(fun trait ->
           sprintf
             "\nimpl%s %s for %s%s {}"
             (type_params_to_string ~bound:trait lifetime tparams)
             trait
             name
             (type_params_to_string lifetime tparams))
    |> String.concat ~sep:""
  in
  match (td.ptype_kind, td.ptype_manifest) with
  | (_, Some ty) ->
    (* The manifest represents a `= <some_type>` clause. When td.ptype_kind is
       Ptype_abstract, this is a simple type alias:

         type foo = Other_module.bar

       In this case, the manifest contains the type Other_module.bar.

       The ptype_kind can also be a full type definition. It is Ptype_variant in
       a declaration like this:

         type foo = Other_module.foo =
            | Bar
            | Baz

       For these declarations, the OCaml compiler verifies that the variants in
       Other_module.foo are equivalent to the ones we define in this
       Ptype_variant.

       I don't think there's a direct equivalent to this in Rust, or any reason
       to try to reproduce it. If we see a manifest, we can ignore the
       ptype_kind and just alias, re-export, or define a newtype for
       Other_module.foo. *)
    (match ty.ptyp_desc with
    (* Polymorphic variants. *)
    | Ptyp_variant _ ->
      raise (Skip_type_decl "polymorphic variants not supported")
    | Ptyp_constr ({ txt = Lident "t"; _ }, []) ->
      (* In the case of `type t = prefix * string ;; type relative_path = t`, we
         have already defined a RelativePath type because we renamed t in the
         first declaration to the name of the module. We can just skip the second
         declaration introducing the alias. *)
      let mod_name_as_type = convert_type_name (curr_module_name ()) in
      if String.equal name mod_name_as_type then
        raise
          (Skip_type_decl
             ("it is an alias to type t, which was already renamed to "
             ^ mod_name_as_type))
      else
        sprintf
          "%s%spub type %s = %s;"
          doc
          attr
          (rust_type name lifetime tparams |> rust_type_to_string)
          mod_name_as_type
    | Ptyp_constr ({ txt = id; _ }, targs) ->
      let id = longident_to_string id in
      let ty_name = id |> String.split ~on:':' |> List.last_exn in
      if
        List.length td.ptype_params = List.length targs
        && String.(self () = ty_name)
        && not mutual_rec
      then (
        add_ty_reexport id;
        raise (Skip_type_decl ("it is a re-export of " ^ id))
      ) else
        let ty = Convert_type.core_type ty in
        if should_add_rcoc name then
          sprintf
            "%s%spub type %s = std::sync::Arc<%s>;"
            doc
            attr
            (rust_type name lifetime tparams |> rust_type_to_string)
            (rust_type_to_string ty)
        else if should_be_newtype name then
          sprintf
            "%s struct %s (%s pub %s);%s\n%s"
            (attrs_and_vis Not_an_enum ~force_derive_copy:false)
            (rust_type name lifetime tparams |> rust_type_to_string)
            (rust_de_field_attr [ty] td.ptype_attributes)
            (rust_type_to_string ty)
            (implements ~force_derive_copy:false)
            (deserialize_in_arena_macro ~force_derive_copy:false)
        else
          sprintf
            "%s%spub type %s = %s;"
            doc
            attr
            (rust_type name lifetime tparams |> rust_type_to_string)
            (deref ty |> rust_type_to_string)
    | _ ->
      if should_use_alias_instead_of_tuple_struct name then
        let ty = Convert_type.core_type ty |> deref |> rust_type_to_string in
        sprintf
          "%s%spub type %s = %s;"
          doc
          attr
          (rust_type name lifetime tparams |> rust_type_to_string)
          ty
      else
        let ty =
          match ty.ptyp_desc with
          | Ptyp_tuple tys ->
            map_and_concat
              ~f:(fun ty ->
                Convert_type.core_type ty |> fun t ->
                sprintf
                  "%s pub %s"
                  (rust_de_field_attr [t] td.ptype_attributes)
                  (rust_type_to_string t))
              ~sep:","
              tys
            |> sprintf "(%s)"
          | _ ->
            Convert_type.core_type ty
            |> rust_type_to_string
            |> sprintf "(pub %s)"
        in
        sprintf
          "%s struct %s %s;%s\n%s"
          (attrs_and_vis Not_an_enum ~force_derive_copy:false)
          (rust_type name lifetime tparams |> rust_type_to_string)
          ty
          (implements ~force_derive_copy:false)
          (deserialize_in_arena_macro ~force_derive_copy:false))
  (* Variant types, including GADTs. *)
  | (Ptype_variant ctors, None) ->
    let all_nullary =
      List.for_all ctors ~f:(fun c -> 0 = ctor_arg_len c.pcd_args)
    in
    let force_derive_copy =
      if is_by_ref () then
        true
      else
        all_nullary
    in
    let box_fields =
      if is_by_ref () then
        true
      else
        should_box_variant name
    in
    let num_variants = List.length ctors in
    let enum_kind =
      if not all_nullary then
        Sum_type { num_variants }
      else
        let max_value =
          ctors
          |> List.filter_map ~f:variant_constructor_value
          |> List.map ~f:int_of_string
          |> List.fold ~init:0 ~f:max
        in
        C_like { max_value; num_variants }
    in
    let ctors =
      map_and_concat ctors ~f:(variant_constructor_declaration ~box_fields)
    in
    sprintf
      "%s enum %s {\n%s}%s\n%s"
      (attrs_and_vis enum_kind ~force_derive_copy)
      (rust_type name lifetime tparams |> rust_type_to_string)
      ctors
      (implements ~force_derive_copy)
      (deserialize_in_arena_macro ~force_derive_copy)
  (* Record types. *)
  | (Ptype_record labels, None) ->
    let prefix = find_record_label_prefix labels in
    let labels = declare_record_arguments labels ~pub:true ~prefix in
    sprintf
      "%s struct %s %s%s\n%s"
      (attrs_and_vis
         Not_an_enum
         ~force_derive_copy:false
         ~additional_attrs:(record_prefix_attr prefix))
      (rust_type name lifetime tparams |> rust_type_to_string)
      labels
      (implements ~force_derive_copy:false)
      (deserialize_in_arena_macro ~force_derive_copy:false)
  (* `type foo`; an abstract type with no specified implementation. This doesn't
     mean much outside of an .mli, I don't think. *)
  | (Ptype_abstract, None) ->
    raise (Skip_type_decl "Abstract types without manifest not supported")
  (* type foo += A, e.g. the exn type. *)
  | (Ptype_open, None) -> raise (Skip_type_decl "Open types not supported")

let type_declaration ?(mutual_rec = false) td =
  let name = td.ptype_name.txt in
  let name =
    if String.equal name "t" then
      curr_module_name ()
    else
      name
  in
  let name = convert_type_name name in
  let name = rename name in
  let mod_name = curr_module_name () in
  if denylisted name then
    log "Not converting type %s::%s: it was denylisted" mod_name name
  else
    match Configuration.extern_type name with
    | Some extern_type ->
      log "Not converting type %s::%s: re-exporting instead" mod_name name;
      add_decl name (sprintf "pub use %s;" extern_type)
    | None ->
      (try
         with_self name (fun () ->
             add_decl name (type_declaration ~mutual_rec name td))
       with
      | Skip_type_decl reason ->
        log "Not converting type %s::%s: %s" mod_name name reason)

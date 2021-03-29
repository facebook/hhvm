(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Asttypes
open Longident
open Parsetree
open Printf
open Utils
open Output
open State
open Convert_longident
open Convert_type

let default_implements () =
  match Configuration.mode () with
  | Configuration.ByRef -> [(Some "arena_trait", "TrivialDrop")]
  | Configuration.ByBox -> []

let implements_traits _name = default_implements ()

let default_derives () =
  (match Configuration.mode () with
  | Configuration.ByBox ->
    [(Some "ocamlrep_derive", "FromOcamlRep"); (Some "serde", "Deserialize")]
  | Configuration.ByRef -> [(Some "ocamlrep_derive", "FromOcamlRepIn")])
  @ [
      (None, "Clone");
      (None, "Debug");
      (None, "Eq");
      (None, "Hash");
      (None, "Ord");
      (None, "PartialEq");
      (None, "PartialOrd");
      (Some "no_pos_hash", "NoPosHash");
      (Some "ocamlrep_derive", "ToOcamlRep");
      (Some "serde", "Serialize");
    ]

let derive_copy ty = Convert_type.is_copy ty ""

let is_by_ref () =
  match Configuration.mode () with
  | Configuration.ByRef -> true
  | Configuration.ByBox -> false

let is_by_box () = not (is_by_ref ())

let additional_derives ty : (string option * string) list =
  ( if derive_copy ty then
    [(None, "Copy")]
  else
    [] )
  @
  match ty with
  | "aast::EmitId" when is_by_box () ->
    [(None, "Copy"); (Some "ocamlrep_derive", "FromOcamlRepIn")]
  | "aast::XhpAttrInfo" when is_by_box () ->
    [(None, "Copy"); (Some "ocamlrep_derive", "FromOcamlRepIn")]
  | _ -> []

let derive_blacklist ty =
  let is_by_ref = is_by_ref () in
  match ty with
  (* A custom implementation of Ord for Error_ matches the sorting behavior of
       errors in OCaml. *)
  | "errors::Error_" -> ["Ord"; "PartialOrd"]
  (* GlobalOptions contains a couple floats, which only implement PartialEq
       and PartialOrd, and do not implement Hash. *)
  | "global_options::GlobalOptions" -> ["Eq"; "Hash"; "NoPosHash"; "Ord"]
  (* And GlobalOptions is used in Genv which is used in Env. We
   * don't care about comparison or hashing on environments *)
  | "typing_env_types::Env" -> ["Eq"; "Hash"; "NoPosHash"; "Ord"]
  | "typing_env_types::Genv" -> ["Eq"; "Hash"; "NoPosHash"; "Ord"]
  | "ast_defs::Id" when is_by_ref -> ["Debug"]
  | "errors::Errors" when is_by_ref -> ["Debug"]
  | "typing_defs_core::Ty" when is_by_ref ->
    ["Eq"; "PartialEq"; "Ord"; "PartialOrd"]
  | "typing_defs_core::Ty_" when is_by_ref -> ["Debug"]
  | "typing_defs_core::ConstraintType" when is_by_ref ->
    ["Eq"; "PartialEq"; "Ord"; "PartialOrd"]
  | _ -> []

let derived_traits ty =
  let ty = sprintf "%s::%s" (curr_module_name ()) ty in
  let blacklist = derive_blacklist ty in
  default_derives ()
  |> List.filter ~f:(fun (_, derive) ->
         not (List.mem blacklist derive ~equal:String.equal))
  |> List.append (additional_derives ty)

let blacklisted_types =
  [
    ("aast_defs", "LocalIdMap");
    ("aast_defs", "ByteString");
    ("decl_defs", "Lin");
    ("decl_defs", "Linearization");
    ("decl_defs", "MroElement");
    ("errors", "FinalizedError");
    ("errors", "Marker");
    ("errors", "MarkedMessage");
    ("errors", "PositionGroup");
    ("typing_defs", "ExpandEnv");
    ("typing_defs", "PhaseTy");
    ("typing_reason", "DeclPhase");
    ("typing_reason", "LoclPhase");
  ]

(* HACK: ignore anything beginning with the "decl" or "locl" prefix, since the
   oxidized version of Ty does not have a phase. *)
let blacklisted_type_prefixes =
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
    ("file_info", "Id");
    ("typing_reason", "PosId");
  ]

(*
A list of (<module>, <ty1>) where ty1 is enum and all non-empty variant fields should
be wrapped by Box to keep ty1 size down.
*)
let box_variant () =
  (match Configuration.mode () with
  | Configuration.ByRef ->
    [("nast", "FuncBodyAnn"); ("typing_defs_core", "Ty_")]
  | Configuration.ByBox -> [])
  @ [("aast", "Expr_"); ("aast", "Stmt_"); ("aast", "Def")]

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
    || (is_prefix ty ~prefix:"Option<" && Convert_type.is_ty_copy ty)
    || (is_prefix ty ~prefix:"std::cell::Cell<" && Convert_type.is_ty_copy ty)
    || (is_prefix ty ~prefix:"std::cell::RefCell<" && Convert_type.is_ty_copy ty)
    || Convert_type.is_primitive ty
  | Configuration.ByBox -> false

let add_rcoc = [("aast", "Nsenv")]

let should_add_rcoc ty =
  match Configuration.mode () with
  | Configuration.ByRef -> false
  | Configuration.ByBox ->
    List.mem add_rcoc (curr_module_name (), ty) ~equal:equal_s2

let blacklisted ty_name =
  let ty = (curr_module_name (), ty_name) in
  List.mem blacklisted_types ty ~equal:equal_s2
  || List.exists blacklisted_type_prefixes ~f:(fun (mod_name, prefix) ->
         String.equal mod_name (curr_module_name ())
         && String.is_prefix ty_name ~prefix)

let rename ty_name =
  List.find renamed_types ~f:(fun (x, _) ->
      equal_s2 x (curr_module_name (), ty_name))
  |> Option.value_map ~f:snd ~default:ty_name

let should_use_alias_instead_of_tuple_struct ty_name =
  List.mem tuple_aliases (curr_module_name (), ty_name) ~equal:( = )

let doc_comment_of_attribute { attr_name; attr_payload; _ } =
  match (attr_name, attr_payload) with
  | ({ txt = "ocaml.doc"; _ }, PStr structure_items) ->
    List.find_map structure_items (fun structure_item ->
        match structure_item.pstr_desc with
        | Pstr_eval
            ({ pexp_desc = Pexp_constant (Pconst_string (doc, _)); _ }, _) ->
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
           if now_in_code_block && was_in_code_block && lstripped = no_asterisk
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

let type_param (ct, _) = core_type ct

let type_params ?bound name params =
  if List.is_empty params then
    match Configuration.mode () with
    | Configuration.ByRef ->
      if Configuration.owned_type name then
        ""
      else
        "<'a>"
    | Configuration.ByBox -> ""
  else
    let bound =
      match bound with
      | None -> ""
      | Some bound -> ": " ^ bound
    in
    let params =
      params |> map_and_concat ~f:(fun tp -> type_param tp ^ bound) ~sep:", "
    in
    match Configuration.mode () with
    | Configuration.ByRef ->
      if Configuration.owned_type name then
        sprintf "<%s>" params
      else
        sprintf "<'a, %s>" params
    | Configuration.ByBox -> sprintf "<%s>" params

let record_label_declaration ?(pub = false) ?(prefix = "") ld =
  let doc = doc_comment_of_attribute_list ld.pld_attributes in
  let pub =
    if pub then
      "pub "
    else
      ""
  in
  let name =
    ld.pld_name.txt |> String.chop_prefix_exn ~prefix |> convert_field_name
  in
  let ty = core_type ld.pld_type in
  sprintf "%s%s%s: %s,\n" doc pub name ty

let record_declaration ?(pub = false) labels =
  let prefix =
    labels |> List.map ~f:(fun ld -> ld.pld_name.txt) |> common_prefix_of_list
  in
  (* Only remove a common prefix up to the last underscore (if a record has
     fields x_bar and x_baz, we want to remove x_, not x_ba). *)
  let prefix =
    let idx = ref (String.length prefix) in
    while !idx > 0 && prefix.[!idx - 1] <> '_' do
      idx := !idx - 1
    done;
    String.sub prefix 0 !idx
  in
  labels
  |> map_and_concat ~f:(record_label_declaration ~pub ~prefix)
  |> sprintf "{\n%s}"

let constructor_arguments ?(box_fields = false) = function
  | Pcstr_tuple types ->
    if not box_fields then
      tuple types
    else (
      match types with
      | [] -> ""
      | [ty] ->
        let ty = core_type ty in
        if unbox_field ty then
          sprintf "(%s)" ty
        else (
          match Configuration.mode () with
          | Configuration.ByRef -> sprintf "(&'a %s)" ty
          | Configuration.ByBox -> sprintf "(Box<%s>)" ty
        )
      | _ ->
        (match Configuration.mode () with
        | Configuration.ByRef ->
          let tys = tuple ~seen_indirection:true types in
          sprintf "(&'a %s)" tys
        | Configuration.ByBox -> sprintf "(Box<%s>)" (tuple types))
    )
  | Pcstr_record labels -> record_declaration labels

let variant_constructor_declaration ?(box_fields = false) cd =
  let doc = doc_comment_of_attribute_list cd.pcd_attributes in
  let name = convert_type_name cd.pcd_name.txt in
  let args = constructor_arguments ~box_fields cd.pcd_args in
  let discriminant =
    (* If we see the [@value 42] attribute, assume it's for ppx_deriving enum,
       and that all the variants are zero-argument (i.e., assume this is a
       C-like enum and provide custom discriminant values). *)
    List.find_map cd.pcd_attributes (fun { attr_name; attr_payload; _ } ->
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
          Some (" = " ^ discriminant)
        | _ -> None)
    |> Option.value ~default:""
  in
  sprintf "%s%s%s%s,\n" doc name args discriminant

let ctor_arg_len (ctor_args : constructor_arguments) : int =
  match ctor_args with
  | Pcstr_tuple x -> List.length x
  | Pcstr_record x -> List.length x

let type_declaration name td =
  let doc = doc_comment_of_attribute_list td.ptype_attributes in
  let attrs_and_vis ~all_nullary ~force_derive_copy =
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
        if all_nullary then
          (Some "ocamlrep_derive", "FromOcamlRep") :: traits
        else
          traits
      in
      let traits =
        if force_derive_copy then
          (Some "ocamlrep_derive", "FromOcamlRepIn") :: traits
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
    doc ^ derive_attr ^ "\npub"
  in
  let tparam_list =
    match (td.ptype_params, td.ptype_name.txt) with
    (* HACK: eliminate tparam from `type _ ty_` and phase-parameterized types *)
    | ([({ ptyp_desc = Ptyp_any; _ }, _)], "ty_")
    | ([({ ptyp_desc = Ptyp_var "phase"; _ }, _)], _)
    | ([({ ptyp_desc = Ptyp_var "ty"; _ }, _)], _)
      when curr_module_name () = "typing_defs_core"
           || curr_module_name () = "typing_defs" ->
      []
    | ([({ ptyp_desc = Ptyp_any; _ }, _)], "t_")
      when curr_module_name () = "typing_reason" ->
      []
    | (tparams, _) -> tparams
  in
  let tparams = type_params name tparam_list in
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
           let tparams_with_bound = type_params name tparam_list ~bound:trait in
           sprintf
             "\nimpl%s %s for %s%s {}"
             tparams_with_bound
             trait
             name
             tparams)
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
      if name = mod_name_as_type then
        raise
          (Skip_type_decl
             ( "it is an alias to type t, which was already renamed to "
             ^ mod_name_as_type ))
      else
        sprintf "%spub type %s%s = %s;" doc name tparams mod_name_as_type
    | Ptyp_constr ({ txt = id; _ }, targs) ->
      let id = longident_to_string id in
      let ty_name = id |> String.split ~on:':' |> List.last_exn in
      if List.length td.ptype_params = List.length targs && self () = ty_name
      then (
        add_ty_reexport id;
        raise (Skip_type_decl ("it is a re-export of " ^ id))
      ) else
        let ty = core_type ty in
        if should_add_rcoc name then
          sprintf
            "%spub type %s%s = ocamlrep::rc::RcOc<%s>;"
            doc
            name
            tparams
            ty
        else
          let ty = Option.value (String.chop_prefix ty "&'a ") ~default:ty in
          sprintf "%spub type %s%s = %s;" doc name tparams ty
    | _ ->
      if should_use_alias_instead_of_tuple_struct name then
        let ty = core_type ty in
        let ty = Option.value (String.chop_prefix ty "&'a ") ~default:ty in
        sprintf "%spub type %s%s = %s;" doc name tparams ty
      else
        let ty =
          match ty.ptyp_desc with
          | Ptyp_tuple tys -> tuple tys ~pub:true
          | _ -> sprintf "(pub %s)" @@ core_type ty
        in
        sprintf
          "%s struct %s%s %s;%s"
          (attrs_and_vis ~all_nullary:false ~force_derive_copy:false)
          name
          tparams
          ty
          (implements ~force_derive_copy:false))
  (* Variant types, including GADTs. *)
  | (Ptype_variant ctors, None) ->
    let all_nullary =
      List.for_all ctors (fun c -> 0 = ctor_arg_len c.pcd_args)
    in
    let force_derive_copy =
      if Configuration.(mode () = ByRef) then
        true
      else
        all_nullary
    in
    let box_fields =
      if Configuration.(mode () = ByRef) then
        true
      else
        should_box_variant name
    in
    let ctors =
      map_and_concat ctors (variant_constructor_declaration ~box_fields)
    in
    sprintf
      "%s enum %s%s {\n%s}%s"
      (attrs_and_vis ~all_nullary ~force_derive_copy)
      name
      tparams
      ctors
      (implements ~force_derive_copy)
  (* Record types. *)
  | (Ptype_record labels, None) ->
    let labels = record_declaration labels ~pub:true in
    sprintf
      "%s struct %s%s %s%s"
      (attrs_and_vis ~all_nullary:false ~force_derive_copy:false)
      name
      tparams
      labels
      (implements ~force_derive_copy:false)
  (* `type foo`; an abstract type with no specified implementation. This doesn't
     mean much outside of an .mli, I don't think. *)
  | (Ptype_abstract, None) ->
    raise (Skip_type_decl "Abstract types without manifest not supported")
  (* type foo += A, e.g. the exn type. *)
  | (Ptype_open, None) -> raise (Skip_type_decl "Open types not supported")

let type_declaration td =
  let name = td.ptype_name.txt in
  let name =
    if name = "t" then
      curr_module_name ()
    else
      name
  in
  let name = convert_type_name name in
  let name = rename name in
  let mod_name = curr_module_name () in
  if blacklisted name then
    log "Not converting type %s::%s: it was blacklisted" mod_name name
  else
    match Configuration.extern_type name with
    | Some extern_type ->
      log "Not converting type %s::%s: re-exporting instead" mod_name name;
      add_decl name (sprintf "pub use %s;" extern_type)
    | None ->
      (try with_self name (fun () -> add_decl name (type_declaration name td))
       with Skip_type_decl reason ->
         log "Not converting type %s::%s: %s" mod_name name reason)

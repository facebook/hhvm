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

let default_derives =
  [
    (None, "Clone");
    (None, "Debug");
    (None, "Eq");
    (None, "Hash");
    (None, "Ord");
    (None, "PartialEq");
    (None, "PartialOrd");
    (Some "ocamlrep_derive", "OcamlRep");
    (Some "serde", "Serialize");
    (Some "serde", "Deserialize");
  ]

let additional_derives : (string option * string) list SMap.t =
  [
    ("typing_tyvar_occurrences::TypingTyvarOccurrences", [(None, "Default")]);
    ("tast::SavedEnv", [(None, "Default")]);
    ("type_parameter_env::TypeParameterEnv", [(None, "Default")]);
    ("typing_inference_env::TypingInferenceEnv", [(None, "Default")]);
  ]
  |> List.fold ~init:SMap.empty ~f:(fun map (ty, x) -> SMap.add ty x map)

let derive_blacklists =
  [
    (* A custom implementation of Ord for Error_ matches the sorting behavior of
       errors in OCaml. *)
    ("errors::Error_", ["Ord"; "PartialOrd"]);
    (* GlobalOptions contains a couple floats, which only implement PartialEq
       and PartialOrd, and do not implement Hash. *)
    ("global_options::GlobalOptions", ["Eq"; "Hash"; "Ord"]);
    (* And GlobalOptions is used in Genv which is used in Env. We
     * don't care about comparison or hashing on environments *)
    ("typing_env_types::Env", ["Eq"; "Hash"; "Ord"]);
    ("typing_env_types::Genv", ["Eq"; "Hash"; "Ord"]);
  ]
  |> List.fold ~init:SMap.empty ~f:(fun map (ty, bl) -> SMap.add ty bl map)

let derived_traits ty =
  let ty = sprintf "%s::%s" (curr_module_name ()) ty in
  begin
    match SMap.find_opt ty derive_blacklists with
    | None -> default_derives
    | Some blacklist ->
      List.filter default_derives ~f:(fun (_, derive) ->
          not (List.mem blacklist derive ~equal:( = )))
  end
  |> List.append
       (Option.value (SMap.find_opt ty additional_derives) ~default:[])

let blacklisted_types =
  [
    ("decl_defs", "Linearization");
    ("typing_defs", "ExpandEnv");
    ("typing_defs", "PhaseTy");
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
let tuple_aliases = [("ast_defs", "Pstring"); ("errors", "Message")]

(*
A list of (<module>, <ty1>) where ty1 is enum and all non-empty variant fields should
be wrapped by Box to keep ty1 size down.
*)
let box_variant = [("aast", "Expr_"); ("aast", "Stmt_"); ("aast", "Def")]

let should_box_variant ty =
  List.mem box_variant (curr_module_name (), ty) ~equal:( = )

let add_rcoc = [("aast", "Nsenv")]

let should_add_rcoc ty =
  List.mem add_rcoc (curr_module_name (), ty) ~equal:( = )

let override_field_type =
  SMap.of_list
    [
      ("Fun_", SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")]);
      ( "Method_",
        SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")] );
      ( "Class_",
        SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")] );
      ( "ClassConst",
        SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")] );
      ( "ClassTypeconst",
        SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")] );
      ( "ClassVar",
        SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")] );
      ( "RecordDef",
        SMap.of_list [("doc_comment", "Option<doc_comment::DocComment>")] );
    ]

let get_overrides name =
  match SMap.find_opt name override_field_type with
  | None -> SMap.empty
  | Some x -> x

let blacklisted ty_name =
  let ty = (curr_module_name (), ty_name) in
  List.mem blacklisted_types ty ~equal:( = )
  || List.exists blacklisted_type_prefixes ~f:(fun (mod_name, prefix) ->
         mod_name = curr_module_name () && String.is_prefix ty_name ~prefix)

let rename ty_name =
  List.find renamed_types ~f:(fun (x, _) -> x = (curr_module_name (), ty_name))
  |> Option.value_map ~f:snd ~default:ty_name

let should_use_alias_instead_of_tuple_struct ty_name =
  List.mem tuple_aliases (curr_module_name (), ty_name) ~equal:( = )

let doc_comment_of_attribute attr =
  match attr with
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

let type_params params =
  if List.is_empty params then
    ""
  else
    params |> map_and_concat ~f:type_param ~sep:", " |> sprintf "<%s>"

let record_label_declaration ?(pub = false) ?(prefix = "") overrides ld =
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
  let ty =
    match SMap.find_opt name overrides with
    | None -> core_type ld.pld_type
    | Some x -> x
  in
  sprintf "%s%s%s: %s,\n" doc pub name ty

let record_declaration ?(pub = false) overrides labels =
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
  |> map_and_concat ~f:(record_label_declaration ~pub ~prefix overrides)
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
        if
          ty = "String"
          || String.is_prefix ty ~prefix:"Vec<"
          || String.is_prefix ty ~prefix:"Block<"
        then
          sprintf "(%s)" ty
        else
          sprintf "(Box<%s>)" ty
      | _ -> sprintf "(Box<%s>)" (tuple types)
    )
  | Pcstr_record labels -> record_declaration SMap.empty labels

let variant_constructor_declaration ?(box_fields = false) cd =
  let doc = doc_comment_of_attribute_list cd.pcd_attributes in
  let name = convert_type_name cd.pcd_name.txt in
  let args = constructor_arguments ~box_fields cd.pcd_args in
  let discriminant =
    (* If we see the [@value 42] attribute, assume it's for ppx_deriving enum,
       and that all the variants are zero-argument (i.e., assume this is a
       C-like enum and provide custom discriminant values). *)
    List.find_map cd.pcd_attributes (fun attr ->
        match attr with
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
  let attrs_and_vis additional_derives =
    let derive_attr =
      derived_traits name @ additional_derives
      |> List.sort ~compare:(fun (_, t1) (_, t2) -> String.compare t1 t2)
      |> List.map ~f:(fun (m, trait) ->
             Option.iter m ~f:(fun m -> add_extern_use (m ^ "::" ^ trait));
             trait)
      |> String.concat ~sep:", "
      |> sprintf "#[derive(%s)]"
    in
    doc ^ derive_attr ^ "\npub"
  in
  let tparams =
    match (td.ptype_params, td.ptype_name.txt) with
    (* HACK: eliminate tparam from `type _ ty_` and phase-parameterized types *)
    | ([({ ptyp_desc = Ptyp_any; _ }, _)], "ty_")
    | ([({ ptyp_desc = Ptyp_var "phase"; _ }, _)], _)
    | ([({ ptyp_desc = Ptyp_var "ty"; _ }, _)], _)
      when curr_module_name () = "typing_defs_core"
           || curr_module_name () = "typing_defs" ->
      ""
    | (tparams, _) -> type_params tparams
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
          sprintf "%spub type %s%s = %s;" doc name tparams ty
    | _ ->
      if should_use_alias_instead_of_tuple_struct name then
        sprintf "%spub type %s%s = %s;" doc name tparams (core_type ty)
      else
        let ty =
          match ty.ptyp_desc with
          | Ptyp_tuple tys -> tuple tys ~pub:true
          | _ -> sprintf "(pub %s)" @@ core_type ty
        in
        sprintf "%s struct %s%s %s;" (attrs_and_vis []) name tparams ty)
  (* Variant types, including GADTs. *)
  | (Ptype_variant ctors, None) ->
    let all_nullary =
      List.for_all ctors (fun c -> 0 = ctor_arg_len c.pcd_args)
    in
    let derives =
      if all_nullary then
        [(None, "Copy")]
      else
        []
    in
    let should_box_variant = should_box_variant name in
    let ctors =
      map_and_concat
        ctors
        (variant_constructor_declaration ~box_fields:should_box_variant)
    in
    sprintf "%s enum %s%s {\n%s}" (attrs_and_vis derives) name tparams ctors
  (* Record types. *)
  | (Ptype_record labels, None) ->
    let labels = record_declaration (get_overrides name) labels ~pub:true in
    sprintf "%s struct %s%s %s" (attrs_and_vis []) name tparams labels
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
    try with_self name (fun () -> add_decl name (type_declaration name td))
    with Skip_type_decl reason ->
      log "Not converting type %s::%s: %s" mod_name name reason

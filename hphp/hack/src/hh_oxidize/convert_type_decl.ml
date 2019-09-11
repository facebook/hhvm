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

let derived_traits =
  [
    (None, "Clone");
    (None, "Debug");
    (Some "ocamlrep_derive", "OcamlRep");
    (Some "ocamlvalue_macro", "Ocamlvalue");
  ]

(* HACK: ignore phases since we are only considering decl tys *)
let blacklisted_types =
  [
    ("typing_defs", "Decl");
    ("typing_defs", "Locl");
    ("typing_defs", "PhaseTy");
  ]

let ocamlvalue_derive_whitelist =
  [
    "aast_defs";
    "aast";
    "ast_defs";
    "namespace_env";
    "file_info";
    "global_options";
    "prim_defs";
  ]

let ocamlvalue_derive_filter (derive : string option * string) : bool =
  snd derive <> "Ocamlvalue"
  || List.mem
       ocamlvalue_derive_whitelist
       ~equal:( = )
       (State.curr_module_name ())

let derives_filters = [ocamlvalue_derive_filter]

(* HACK: Typing_reason is usually aliased to Reason, so we have lots of
   instances of Reason.t. Since we usually convert an identifier like Reason.t
   to reason::Reason, the actual type needs to be renamed to the common alias.
   This looks nicer anyway. *)
let renamed_types = [(("typing_reason", "TypingReason"), "Reason")]

(* By default, when we see an alias to a tuple type, we will assume the alias
   adds some meaning, and generate a new tuple struct type named after the
   alias. In some cases, the alias adds no meaning and we should also use an
   alias in Rust. *)
let tuple_aliases = [("ast_defs", "Pstring")]

let blacklisted ty_name =
  List.mem blacklisted_types (curr_module_name (), ty_name) ~equal:( = )

let rename ty_name =
  List.find renamed_types ~f:(fun (x, _) -> x = (curr_module_name (), ty_name))
  |> Option.value_map ~f:snd ~default:ty_name

let should_use_alias_instead_of_tuple_struct ty_name =
  List.mem tuple_aliases (curr_module_name (), ty_name) ~equal:( = )

let type_param (ct, _) = core_type ct

let type_params params =
  if List.is_empty params then
    ""
  else
    params |> map_and_concat ~f:type_param ~sep:", " |> sprintf "<%s>"

let record_label_declaration ?(pub = false) ?(prefix = "") ld =
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
  sprintf "%s%s: %s,\n" pub name ty

let record_declaration ?(pub = false) labels =
  let prefix =
    labels |> List.map ~f:(fun ld -> ld.pld_name.txt) |> common_prefix_of_list
  in
  (* Only remove a common prefix up to the last underscore (if a record has
     fields x_bar and x_baz, we want to remove x_, not x_ba). *)
  let prefix =
    let idx = ref (String.length prefix) in
    while !idx > 1 && prefix.[!idx - 1] <> '_' do
      idx := !idx - 1
    done;
    String.sub prefix 0 !idx
  in
  labels
  |> map_and_concat ~f:(record_label_declaration ~pub ~prefix)
  |> sprintf "{\n%s}"

let constructor_arguments = function
  | Pcstr_tuple types -> tuple types
  | Pcstr_record labels -> record_declaration labels

let variant_constructor_declaration cd =
  let name = convert_type_name cd.pcd_name.txt in
  let args = constructor_arguments cd.pcd_args in
  sprintf "%s%s,\n" name args

let ctor_arg_len (ctor_args : constructor_arguments) : int =
  match ctor_args with
  | Pcstr_tuple x -> List.length x
  | Pcstr_record x -> List.length x

let type_declaration name td =
  let attrs_and_vis init_derives =
    let filter a f = List.filter a ~f in
    let derive_attr =
      List.fold derives_filters ~init:(derived_traits @ init_derives) ~f:filter
      |> List.sort ~compare:(fun (_, t1) (_, t2) -> String.compare t1 t2)
      |> List.map ~f:(fun (m, trait) ->
             Option.iter m ~f:(fun m -> add_extern_use (m ^ "::" ^ trait));
             trait)
      |> String.concat ~sep:", "
      |> sprintf "#[derive(%s)]"
    in
    derive_attr ^ "\npub"
  in
  let tparams =
    match (td.ptype_params, td.ptype_name.txt) with
    (* HACK: eliminate tparam from `type _ ty_` and phase-parameterized types *)
    | ([({ ptyp_desc = Ptyp_any; _ }, _)], "ty_")
    | ([({ ptyp_desc = Ptyp_var "phase"; _ }, _)], _) ->
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
    (* In the case of `type t = prefix * string ;; type relative_path = t`, we
       have already defined a RelativePath type because we renamed t in the
       first declaration to the name of the module. We can just skip the second
       declaration introducing the alias. *)
    | Ptyp_constr ({ txt = Lident "t"; _ }, []) ->
      raise (Skip_type_decl "it is an alias to type t, which was renamed")
    | Ptyp_constr ({ txt = id; _ }, targs) ->
      let id = longident_to_string id in
      let ty_name = id |> String.split ~on:':' |> List.last_exn in
      if List.length td.ptype_params = List.length targs && self () = ty_name
      then (
        add_ty_use id ty_name;
        raise (Skip_type_decl ("it is a re-export of " ^ id))
      ) else
        sprintf "pub type %s%s = %s;" name tparams (core_type ty)
    | _ ->
      if should_use_alias_instead_of_tuple_struct name then
        sprintf "pub type %s%s = %s;" name tparams (core_type ty)
      else
        let ty =
          match ty.ptyp_desc with
          | Ptyp_tuple tys -> tuple tys ~pub:true
          | _ -> sprintf "(pub %s)" @@ core_type ty
        in
        sprintf "%s struct %s%s %s;" (attrs_and_vis []) name tparams ty)
  (* Variant types, including GADTs. *)
  | (Ptype_variant ctors, None) ->
    let ctors =
      (* HACK: consider only decl tys for now by eliminating locl ty variants *)
      if name <> "Ty_" then
        ctors
      else
        List.filter ctors (fun cd ->
            match cd.pcd_res with
            | Some
                {
                  ptyp_desc =
                    Ptyp_constr
                      ( { txt = Lident "ty_"; _ },
                        [
                          {
                            ptyp_desc =
                              Ptyp_constr ({ txt = Lident "locl"; _ }, _);
                            _;
                          };
                        ] );
                  _;
                } ->
              log
                "Not generating an equivalent to the locl ty_ constructor %s"
                cd.pcd_name.txt;
              false
            | _ -> true)
    in
    let all_nullary =
      List.for_all ctors (fun c -> 0 = ctor_arg_len c.pcd_args)
    in
    let derives =
      if all_nullary then
        [(None, "Copy"); (None, "Eq"); (None, "PartialEq")]
      else
        []
    in
    let ctors = map_and_concat ctors variant_constructor_declaration in
    sprintf "%s enum %s%s {\n%s}" (attrs_and_vis derives) name tparams ctors
  (* Record types. *)
  | (Ptype_record labels, None) ->
    let labels = record_declaration labels ~pub:true in
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

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Asttypes
open Parsetree
open Utils
open Output
open Convert_longident

(* HACK: These modules are not used in any type declarations, so importing them
   will result in an "unused import" warning or an "unresolved import" error in
   Rust. *)
let module_blacklist =
  [
    (* nast.ml opens Aast, but doesn't use it in type declarations. *)
      "aast";
    "aast_defs_visitors_ancestors";
    "ast_defs_visitors_ancestors";
    "hh_core";
    "naming_special_names";
    "pp_type";
    "reordered_argument_collections";
    "string_utils";
    "utils";
    "core_kernel";
  ]

(* HACK: These submodules are defined inline in another module. We don't convert
   nested modules (I think it would be a bit of work to handle imports
   properly), so we convert them manually and re-export them when we see their
   OCaml definition. *)
let nested_modules =
  [("ast_defs", "ShapeMap"); ("global_options", "InferMissing")]

let blacklisted = List.mem module_blacklist ~equal:String.equal

let is_manually_converted_nested_module mod_name =
  List.mem
    nested_modules
    (State.curr_module_name (), mod_name)
    ~equal:(Tuple.T2.equal ~eq1:String.equal ~eq2:String.equal)

let string_of_module_desc = function
  | Pmod_structure _ -> "Pmod_structure"
  | Pmod_functor _ -> "Pmod_functor"
  | Pmod_apply _ -> "Pmod_apply"
  | Pmod_constraint _ -> "Pmod_constraint"
  | Pmod_unpack _ -> "Pmod_unpack"
  | Pmod_extension _ -> "Pmod_extension"
  | Pmod_ident _ -> "Pmod_ident"

let structure_item si =
  match si.pstr_desc with
  (* A type declaration. The list type_decls will contain multiple items in the
     event of `type ... and`. *)
  | Pstr_type (_, type_decls) ->
    List.iter type_decls Convert_type_decl.type_declaration
  (* Convert `open Foo` to `use crate::foo::*;` *)
  | Pstr_open { popen_lid = id; _ } ->
    let mod_name = longident_to_string id.txt ~for_open:true in
    if blacklisted mod_name then
      log "Not opening %s: it is blacklisted" mod_name
    else
      add_glob_use mod_name
  (* Convert `module F = Foo` to `use crate::foo as f;` *)
  | Pstr_module
      {
        pmb_name = { txt = alias; _ };
        pmb_expr = { pmod_desc = Pmod_ident id; _ };
        _;
      } ->
    let mod_name = longident_to_string id.txt ~for_open:true in
    if blacklisted mod_name then
      log "Not aliasing %s: it is blacklisted" mod_name
    else
      add_alias mod_name (convert_module_name alias)
  (* Convert `include Foo` to explicit re-exports (`pub use`) for every type
     exported by Foo (see {!Stringify.get_includes}). *)
  | Pstr_include { pincl_mod = { pmod_desc = Pmod_ident id; _ }; _ } ->
    let mod_name = longident_to_string id.txt ~for_open:true in
    if blacklisted mod_name then
      log "Not including %s: it is blacklisted" mod_name
    else
      add_include mod_name
  | Pstr_module
      {
        pmb_name = { txt = mod_name; _ };
        pmb_expr = { pmod_desc = Pmod_structure _; _ };
        _;
      }
    when is_manually_converted_nested_module mod_name ->
    let rust_mod_name = convert_module_name mod_name in
    log
      "Not converting submodule %s: importing crate::%s instead"
      mod_name
      rust_mod_name;
    add_alias rust_mod_name rust_mod_name
  | Pstr_module
      { pmb_name = { txt = name; _ }; pmb_expr = { pmod_desc; _ }; _ } ->
    let kind = string_of_module_desc pmod_desc in
    log "Not converting submodule %s: %s not supported" name kind
  | Pstr_include { pincl_mod = { pmod_desc; _ }; _ } ->
    let kind = string_of_module_desc pmod_desc in
    log "Not converting include: %s not supported" kind
  | Pstr_exception { pext_name = { txt = name; _ }; _ } ->
    log "Not converting exception %s" name
  | Pstr_eval _ -> log "Not converting Pstr_eval"
  | Pstr_primitive _ -> log "Not converting Pstr_primitive"
  | Pstr_typext _ -> log "Not converting Pstr_typext"
  | Pstr_recmodule _ -> log "Not converting Pstr_recmodule"
  | Pstr_modtype _ -> log "Not converting Pstr_modtype"
  | Pstr_class _ -> log "Not converting Pstr_class"
  | Pstr_class_type _ -> log "Not converting Pstr_class_type"
  | Pstr_extension _ -> log "Not converting Pstr_extension"
  (* Doc comments on files are represented with Pstr_attribute, so silently
     ignore them. *)
  | Pstr_attribute _ -> ()
  (* Our goal is to convert types only, so silently ignore values. *)
  | Pstr_value _ -> ()

let toplevel_phrase = function
  | Ptop_def items -> List.iter items structure_item
  | Ptop_dir _ -> log "Not converting toplevel directive"

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)

open Hh_prelude
open Common
open String_utils
open Naming_phase_sigs

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)
let elaborate_namespaces =
  new Naming_elaborate_namespaces_endo.generic_elaborator

let invalid_expr_ = Naming_phase_error.invalid_expr_

(**************************************************************************)
(* The entry points to CHECK the program, and transform the program *)
(**************************************************************************)

type 'elem pipeline = {
  elab_ns: 'elem -> 'elem;
  elab_happly: (Naming_elab_happly_hint.Env.t, 'elem) elabidation;
  elab_retonly: (Naming_elab_retonly_hint.Env.t, 'elem) elabidation;
  elab_wildcard: (Naming_elab_wildcard_hint.Env.t, 'elem) elabidation;
  validate_like: (Naming_validate_like_hint.Env.t, 'elem) validation;
  elab_shape_field_name:
    (Naming_elab_shape_field_name.Env.t, 'elem) elabidation;
  elab_haccess: (Naming_elab_haccess_hint.Env.t, 'elem) elabidation;
  elab_this: (Naming_elab_this_hint.Env.t, 'elem) elabidation;
  validate_cast: (Naming_validate_cast_expr.Env.t, 'elem) validation;
  elab_collection: (Naming_elab_collection.Env.t, 'elem) elabidation;
  elab_call: (Naming_elab_call.Env.t, 'elem) elabidation;
  elab_tuple: (Naming_elab_tuple.Env.t, 'elem) elabidation;
  elab_invariant: (Naming_elab_invariant.Env.t, 'elem) elabidation;
  elab_const_expr: (Naming_elab_const_expr.Env.t, 'elem) elabidation;
  elab_user_attrs: (Naming_elab_user_attributes.Env.t, 'elem) elabidation;
  elab_import: (Naming_elab_import.Env.t, 'elem) elaboration;
  elab_lvar: (Naming_elab_lvar.Env.t, 'elem) elaboration;
  elab_as_expr: (Naming_elab_as_expr.Env.t, 'elem) elabidation;
  elab_block: (Naming_elab_block.Env.t, 'elem) elaboration;
  elab_pipe: (Naming_elab_pipe.Env.t, 'elem) elaboration;
  elab_func_body: (Naming_elab_func_body.Env.t, 'elem) elaboration;
  elab_lambda_captures: (Naming_captures.Env.t, 'elem) elaboration;
  elab_class_members: (Naming_elab_class_members.Env.t, 'elem) elabidation;
  elab_defs: (Naming_elab_defs.Env.t, 'elem) elaboration;
  elab_soft: (Naming_elab_soft.Env.t, 'elem) elaboration;
  elab_everything_sdt: (Naming_elab_everything_sdt.Env.t, 'elem) elaboration;
  elab_hkt: (Naming_elab_hkt.Env.t, 'elem) elabidation;
  elab_enum_class: (Naming_elab_enum_class.Env.t, 'elem) elaboration;
  elab_class_id: (Naming_elab_class_id.Env.t, 'elem) elabidation;
  elab_dynamic_class_name:
    (Naming_elab_dynamic_class_name.Env.t, 'elem) elabidation;
  validate_fun_params: (Naming_validate_fun_params.Env.t, 'elem) validation;
  validate_xhp: (Naming_validate_xhp_name.Env.t, 'elem) validation;
  validate_builtin_enum: (Naming_validate_builtin_enum.Env.t, 'elem) validation;
  validate_enum_class_typeconst:
    (Naming_validate_enum_class_typeconst.Env.t, 'elem) validation;
  validate_supportdyn: (Naming_validate_supportdyn.Env.t, 'elem) validation;
  validate_module: (Naming_validate_module.Env.t, 'elem) validation;
  validate_class_req: (Naming_validate_class_req.Env.t, 'elem) validation;
  validate_consistent_construct:
    (Naming_validate_consistent_construct.Env.t, 'elem) validation;
}

(* Apply our elaboration and validation steps to a given ast element *)
let elab_elem
    elem
    ~ctx
    ~filename
    {
      elab_ns;
      elab_defs;
      elab_happly;
      elab_retonly;
      elab_wildcard;
      validate_like;
      elab_shape_field_name;
      elab_haccess;
      elab_this;
      validate_cast;
      elab_collection;
      elab_call;
      elab_tuple;
      elab_invariant;
      elab_const_expr;
      elab_user_attrs;
      elab_import;
      elab_lvar;
      elab_as_expr;
      elab_block;
      elab_pipe;
      elab_func_body;
      elab_lambda_captures;
      elab_class_members;
      elab_soft;
      elab_everything_sdt;
      elab_hkt;
      elab_enum_class;
      elab_class_id;
      elab_dynamic_class_name;
      validate_fun_params;
      validate_xhp;
      validate_builtin_enum;
      validate_enum_class_typeconst;
      validate_supportdyn;
      validate_module;
      validate_class_req;
      validate_consistent_construct;
    } =
  let tcopt = Provider_context.get_tcopt ctx in
  (* Elaborate namespaces *)
  let elem = elab_ns elem in

  (* Remove or flatten top level defs *)
  let elem = elab_defs elem in

  (* Elaborate / validate hints *)
  let (elem, err) = elab_happly elem in
  let (elem, err) = elab_retonly ~init:err elem in
  let (elem, err) = elab_wildcard ~init:err elem in
  let err = validate_like ~init:err elem in
  let (elem, err) = elab_shape_field_name ~init:err elem in
  let (elem, err) = elab_this ~init:err elem in
  let (elem, err) = elab_haccess ~init:err elem in
  let err = validate_cast ~init:err elem in

  (* Check for invalid use of internal classes - note that we must have this
     validation pass _before_ we elaborate enum classes *)
  let err =
    if
      not
        (string_ends_with (Relative_path.suffix filename) ".hhi"
        || TypecheckerOptions.is_systemlib (Provider_context.get_tcopt ctx))
    then
      validate_builtin_enum ~init:err elem
    else
      err
  in

  (* Add implicit extends for enums & enum classes *)
  let elem = elab_enum_class elem in

  (* Use canonical class_ids and validate usage of special names outside classes *)
  let (elem, err) = elab_class_id ~init:err elem in

  (* Warn and rewrite dynamic class_id expressions to unknown class ident
     NB: this must be sequenced after class_id elaboration so that class_id's
     are in canonical form, specifically that any remaining `CIexpr` represent
     dynamic new's
  *)
  let (elem, err) = elab_dynamic_class_name ~init:err elem in

  (* Elaboration `Collection` to `ValCollection` or `KeyValCollection *)
  let (elem, err) = elab_collection ~init:err elem in

  let (elem, err) = elab_call ~init:err elem in

  let (elem, err) = elab_tuple ~init:err elem in

  let (elem, err) = elab_invariant ~init:err elem in

  let (elem, err) = elab_const_expr ~init:err elem in

  let (elem, err) = elab_user_attrs ~init:err elem in

  let elem = elab_import elem in

  let elem = elab_lvar elem in

  let (elem, err) = elab_as_expr ~init:err elem in

  let err = validate_fun_params ~init:err elem in

  let elem = elab_block elem in

  let elem = elab_pipe elem in

  let elem = elab_func_body elem in

  let elem = elab_lambda_captures elem in

  let err = validate_class_req ~init:err elem in

  let err =
    let level = TypecheckerOptions.explicit_consistent_constructors tcopt in
    if level > 0 then
      let env = Naming_validate_consistent_construct.Env.create level in
      validate_consistent_construct ~init:err ~env elem
    else
      err
  in

  let (elem, err) = elab_class_members ~init:err elem in

  (* Miscellaneous validation  *)
  (* TODO[mjt] move these to NAST checks*)
  (* Check for specific errors when referring to xhp classes *)
  let err = validate_xhp ~init:err elem in

  (* Apply elaboration / validation based on typechecker options *)
  (* Soft types *)
  let soft_as_like =
    TypecheckerOptions.interpret_soft_types_as_like_types tcopt
  in
  let elem = elab_soft elem ~env:soft_as_like in

  (* If HKTs are not enabled, we remove type parameter here and generate
     specific errors rather than arity errors in typing *)
  (* TODO[mjt] you do get an arity error from typing if you don't do
     this - we might consider specializing _that_ error to give info
     about HKTs rather than doing this from naming *)
  let hkt_enabled = TypecheckerOptions.higher_kinded_types tcopt in
  let (elem, err) =
    if not hkt_enabled then
      elab_hkt ~init:err elem
    else
      (elem, err)
  in

  (* SupportDyn *)
  let err =
    if
      (not
         (string_ends_with (Relative_path.suffix filename) ".hhi"
         || TypecheckerOptions.is_systemlib tcopt))
      && not
           (TypecheckerOptions.experimental_feature_enabled
              tcopt
              TypecheckerOptions.experimental_supportdynamic_type_hint)
    then
      validate_supportdyn ~init:err elem
    else
      err
  in

  (* Sound dynamic *)
  let elem =
    if TypecheckerOptions.everything_sdt tcopt then
      elab_everything_sdt elem
    else
      elem
  in

  (* enum class type constants *)
  (* we're still in the middle of developing type constants for enum classes
     so we gate them carefully for now:
     They must use the feature flag `type_constants_in_enum_class` AND
     be in a selected list of directories.

     For internal testing, we provide a global "enable" flag to just
     enable them. This is off by default except in hh_single_type_check.
  *)
  let file_str = Relative_path.suffix filename in
  let dir_str = Filename.dirname file_str in

  let err =
    if
      TypecheckerOptions.allow_all_locations_for_type_constant_in_enum_class
        tcopt
      || List.exists ~f:(String.equal dir_str)
         @@ TypecheckerOptions.allowed_locations_for_type_constant_in_enum_class
              tcopt
    then
      err
    else
      validate_enum_class_typeconst ~init:err elem
  in

  (* modules *)
  let err =
    if
      TypecheckerOptions.allow_all_files_for_module_declarations tcopt
      || List.exists
           ~f:(fun allowed_file ->
             let len = String.length allowed_file in
             if len > 0 then
               match allowed_file.[len - 1] with
               | '*' ->
                 let allowed_dir =
                   String.sub allowed_file ~pos:0 ~len:(len - 1)
                 in
                 String_utils.string_starts_with file_str allowed_dir
               | _ -> String.equal allowed_file file_str
             else
               false)
           (TypecheckerOptions.allowed_files_for_module_declarations tcopt)
    then
      err
    else
      validate_module ~init:err elem
  in

  (* We have to check if like-types are globally enabled and remove errors
     before reporting if so *)
  let err =
    if TypecheckerOptions.like_type_hints tcopt then
      Naming_phase_error.suppress_like_type_errors err
    else
      err
  in
  Naming_phase_error.emit err;
  elem

let program_filename defs =
  let open Aast_defs in
  let rec aux = function
    | Fun fun_def :: _ -> Pos.filename fun_def.fd_fun.f_span
    | Class class_ :: _ -> Pos.filename class_.c_span
    | Stmt (pos, _) :: _ -> Pos.filename pos
    | Typedef typedef :: _ -> Pos.filename typedef.t_span
    | Constant gconst :: _ -> Pos.filename gconst.cst_span
    | Module module_def :: _ -> Pos.filename module_def.md_span
    | _ :: rest -> aux rest
    | _ -> Relative_path.default
  in
  aux defs

let program ctx ast =
  let filename = program_filename ast in
  elab_elem
    ast
    ~ctx
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_program
          (Naming_elaborate_namespaces_endo.make_env
             Namespace_env.empty_with_default);
      elab_defs = Naming_elab_defs.elab_program;
      elab_happly = Naming_elab_happly_hint.elab_program;
      elab_retonly = Naming_elab_retonly_hint.elab_program;
      elab_wildcard = Naming_elab_wildcard_hint.elab_program;
      elab_shape_field_name = Naming_elab_shape_field_name.elab_program;
      validate_like = Naming_validate_like_hint.validate_program;
      elab_this = Naming_elab_this_hint.elab_program;
      elab_haccess = Naming_elab_haccess_hint.elab_program;
      validate_cast = Naming_validate_cast_expr.validate_program;
      elab_collection = Naming_elab_collection.elab_program;
      elab_call = Naming_elab_call.elab_program;
      elab_tuple = Naming_elab_tuple.elab_program;
      elab_invariant = Naming_elab_invariant.elab_program;
      elab_const_expr = Naming_elab_const_expr.elab_program;
      elab_user_attrs = Naming_elab_user_attributes.elab_program;
      elab_import = Naming_elab_import.elab_program;
      elab_lvar = Naming_elab_lvar.elab_program;
      elab_as_expr = Naming_elab_as_expr.elab_program;
      elab_block = Naming_elab_block.elab_program;
      elab_pipe = Naming_elab_pipe.elab_program;
      elab_func_body = Naming_elab_func_body.elab_program;
      elab_lambda_captures = Naming_captures.elab_program;
      elab_class_members = Naming_elab_class_members.elab_program;
      elab_soft = Naming_elab_soft.elab_program;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_program;
      elab_hkt = Naming_elab_hkt.elab_program;
      elab_enum_class = Naming_elab_enum_class.elab_program;
      elab_class_id = Naming_elab_class_id.elab_program;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_program;
      validate_fun_params = Naming_validate_fun_params.validate_program;
      validate_xhp = Naming_validate_xhp_name.validate_program;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_program;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_program;
      validate_supportdyn = Naming_validate_supportdyn.validate_program;
      validate_module = Naming_validate_module.validate_program;
      validate_class_req = Naming_validate_class_req.validate_program;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_program;
    }

let fun_def ctx fd =
  let filename = Pos.filename fd.Aast.fd_fun.Aast.f_span in
  elab_elem
    fd
    ~ctx
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_fun_def
          (Naming_elaborate_namespaces_endo.make_env fd.Aast.fd_namespace);
      elab_defs = Naming_elab_defs.elab_fun_def;
      elab_happly = Naming_elab_happly_hint.elab_fun_def;
      elab_retonly = Naming_elab_retonly_hint.elab_fun_def;
      elab_wildcard = Naming_elab_wildcard_hint.elab_fun_def;
      validate_like = Naming_validate_like_hint.validate_fun_def;
      elab_shape_field_name = Naming_elab_shape_field_name.elab_fun_def;
      elab_this = Naming_elab_this_hint.elab_fun_def;
      elab_haccess = Naming_elab_haccess_hint.elab_fun_def;
      validate_cast = Naming_validate_cast_expr.validate_fun_def;
      elab_collection = Naming_elab_collection.elab_fun_def;
      elab_call = Naming_elab_call.elab_fun_def;
      elab_tuple = Naming_elab_tuple.elab_fun_def;
      elab_invariant = Naming_elab_invariant.elab_fun_def;
      elab_const_expr = Naming_elab_const_expr.elab_fun_def;
      elab_user_attrs = Naming_elab_user_attributes.elab_fun_def;
      elab_import = Naming_elab_import.elab_fun_def;
      elab_lvar = Naming_elab_lvar.elab_fun_def;
      elab_as_expr = Naming_elab_as_expr.elab_fun_def;
      elab_block = Naming_elab_block.elab_fun_def;
      elab_pipe = Naming_elab_pipe.elab_fun_def;
      elab_func_body = Naming_elab_func_body.elab_fun_def;
      elab_lambda_captures = Naming_captures.elab_fun_def;
      elab_class_members = Naming_elab_class_members.elab_fun_def;
      elab_soft = Naming_elab_soft.elab_fun_def;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_fun_def;
      elab_hkt = Naming_elab_hkt.elab_fun_def;
      elab_enum_class = Naming_elab_enum_class.elab_fun_def;
      elab_class_id = Naming_elab_class_id.elab_fun_def;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_fun_def;
      validate_fun_params = Naming_validate_fun_params.validate_fun_def;
      validate_xhp = Naming_validate_xhp_name.validate_fun_def;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_fun_def;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_fun_def;
      validate_supportdyn = Naming_validate_supportdyn.validate_fun_def;
      validate_module = Naming_validate_module.validate_fun_def;
      validate_class_req = Naming_validate_class_req.validate_fun_def;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_fun_def;
    }

let class_ ctx c =
  let filename = Pos.filename c.Aast.c_span in
  elab_elem
    c
    ~ctx
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_class_
          (Naming_elaborate_namespaces_endo.make_env c.Aast.c_namespace);
      elab_defs = Naming_elab_defs.elab_class;
      elab_retonly = Naming_elab_retonly_hint.elab_class;
      elab_happly = Naming_elab_happly_hint.elab_class;
      elab_wildcard = Naming_elab_wildcard_hint.elab_class;
      validate_like = Naming_validate_like_hint.validate_class;
      elab_shape_field_name = Naming_elab_shape_field_name.elab_class;
      elab_this = Naming_elab_this_hint.elab_class;
      elab_haccess = Naming_elab_haccess_hint.elab_class;
      validate_cast = Naming_validate_cast_expr.validate_class;
      elab_collection = Naming_elab_collection.elab_class;
      elab_call = Naming_elab_call.elab_class;
      elab_tuple = Naming_elab_tuple.elab_class;
      elab_invariant = Naming_elab_invariant.elab_class;
      elab_const_expr = Naming_elab_const_expr.elab_class;
      elab_user_attrs = Naming_elab_user_attributes.elab_class;
      elab_import = Naming_elab_import.elab_class;
      elab_lvar = Naming_elab_lvar.elab_class;
      elab_as_expr = Naming_elab_as_expr.elab_class;
      elab_block = Naming_elab_block.elab_class;
      elab_pipe = Naming_elab_pipe.elab_class;
      elab_func_body = Naming_elab_func_body.elab_class;
      elab_lambda_captures = Naming_captures.elab_class;
      elab_class_members = Naming_elab_class_members.elab_class;
      elab_soft = Naming_elab_soft.elab_class;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_class;
      elab_hkt = Naming_elab_hkt.elab_class;
      elab_enum_class = Naming_elab_enum_class.elab_class;
      elab_class_id = Naming_elab_class_id.elab_class;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_class;
      validate_fun_params = Naming_validate_fun_params.validate_class;
      validate_xhp = Naming_validate_xhp_name.validate_class;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_class;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_class;
      validate_supportdyn = Naming_validate_supportdyn.validate_class;
      validate_module = Naming_validate_module.validate_class;
      validate_class_req = Naming_validate_class_req.validate_class;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_class;
    }

let module_ ctx module_ =
  let filename = Pos.filename module_.Aast.md_span in
  elab_elem
    module_
    ~ctx
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_module_def
          (Naming_elaborate_namespaces_endo.make_env
             Namespace_env.empty_with_default);
      elab_defs = Naming_elab_defs.elab_module_def;
      elab_retonly = Naming_elab_retonly_hint.elab_module_def;
      elab_happly = Naming_elab_happly_hint.elab_module_def;
      elab_wildcard = Naming_elab_wildcard_hint.elab_module_def;
      elab_shape_field_name = Naming_elab_shape_field_name.elab_module_def;
      validate_like = Naming_validate_like_hint.validate_module_def;
      elab_this = Naming_elab_this_hint.elab_module_def;
      elab_haccess = Naming_elab_haccess_hint.elab_module_def;
      validate_cast = Naming_validate_cast_expr.validate_module_def;
      elab_collection = Naming_elab_collection.elab_module_def;
      elab_call = Naming_elab_call.elab_module_def;
      elab_tuple = Naming_elab_tuple.elab_module_def;
      elab_invariant = Naming_elab_invariant.elab_module_def;
      elab_const_expr = Naming_elab_const_expr.elab_module_def;
      elab_user_attrs = Naming_elab_user_attributes.elab_module_def;
      elab_import = Naming_elab_import.elab_module_def;
      elab_lvar = Naming_elab_lvar.elab_module_def;
      elab_as_expr = Naming_elab_as_expr.elab_module_def;
      elab_block = Naming_elab_block.elab_module_def;
      elab_pipe = Naming_elab_pipe.elab_module_def;
      elab_func_body = Naming_elab_func_body.elab_module_def;
      elab_lambda_captures = Naming_captures.elab_module_def;
      elab_class_members = Naming_elab_class_members.elab_module_def;
      elab_soft = Naming_elab_soft.elab_module_def;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_module_def;
      elab_hkt = Naming_elab_hkt.elab_module_def;
      elab_enum_class = Naming_elab_enum_class.elab_module_def;
      elab_class_id = Naming_elab_class_id.elab_module_def;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_module_def;
      validate_fun_params = Naming_validate_fun_params.validate_module_def;
      validate_xhp = Naming_validate_xhp_name.validate_module_def;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_module_def;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_module_def;
      validate_supportdyn = Naming_validate_supportdyn.validate_module_def;
      validate_module = Naming_validate_module.validate_module_def;
      validate_class_req = Naming_validate_class_req.validate_module_def;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_module_def;
    }

let global_const ctx cst =
  let filename = Pos.filename cst.Aast.cst_span in
  elab_elem
    cst
    ~ctx
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_gconst
          (Naming_elaborate_namespaces_endo.make_env cst.Aast.cst_namespace);
      elab_defs = Naming_elab_defs.elab_gconst;
      elab_retonly = Naming_elab_retonly_hint.elab_gconst;
      elab_happly = Naming_elab_happly_hint.elab_gconst;
      elab_wildcard = Naming_elab_wildcard_hint.elab_gconst;
      validate_like = Naming_validate_like_hint.validate_gconst;
      elab_shape_field_name = Naming_elab_shape_field_name.elab_gconst;
      elab_this = Naming_elab_this_hint.elab_gconst;
      elab_haccess = Naming_elab_haccess_hint.elab_gconst;
      validate_cast = Naming_validate_cast_expr.validate_gconst;
      elab_collection = Naming_elab_collection.elab_gconst;
      elab_call = Naming_elab_call.elab_gconst;
      elab_tuple = Naming_elab_tuple.elab_gconst;
      elab_invariant = Naming_elab_invariant.elab_gconst;
      elab_const_expr = Naming_elab_const_expr.elab_gconst;
      elab_user_attrs = Naming_elab_user_attributes.elab_gconst;
      elab_import = Naming_elab_import.elab_gconst;
      elab_lvar = Naming_elab_lvar.elab_gconst;
      elab_as_expr = Naming_elab_as_expr.elab_gconst;
      elab_block = Naming_elab_block.elab_gconst;
      elab_pipe = Naming_elab_pipe.elab_gconst;
      elab_func_body = Naming_elab_func_body.elab_gconst;
      elab_lambda_captures = Naming_captures.elab_gconst;
      elab_class_members = Naming_elab_class_members.elab_gconst;
      elab_soft = Naming_elab_soft.elab_gconst;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_gconst;
      elab_hkt = Naming_elab_hkt.elab_gconst;
      elab_enum_class = Naming_elab_enum_class.elab_gconst;
      elab_class_id = Naming_elab_class_id.elab_gconst;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_gconst;
      validate_fun_params = Naming_validate_fun_params.validate_gconst;
      validate_xhp = Naming_validate_xhp_name.validate_gconst;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_gconst;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_gconst;
      validate_supportdyn = Naming_validate_supportdyn.validate_gconst;
      validate_module = Naming_validate_module.validate_gconst;
      validate_class_req = Naming_validate_class_req.validate_gconst;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_gconst;
    }

let typedef ctx tdef =
  let filename = Pos.filename @@ tdef.Aast.t_span in
  elab_elem
    tdef
    ~ctx
    ~filename
    {
      elab_ns =
        elaborate_namespaces#on_typedef
          (Naming_elaborate_namespaces_endo.make_env tdef.Aast.t_namespace);
      elab_defs = Naming_elab_defs.elab_typedef;
      elab_retonly = Naming_elab_retonly_hint.elab_typedef;
      elab_happly = Naming_elab_happly_hint.elab_typedef;
      elab_wildcard = Naming_elab_wildcard_hint.elab_typedef;
      validate_like = Naming_validate_like_hint.validate_typedef;
      elab_shape_field_name = Naming_elab_shape_field_name.elab_typedef;
      elab_this = Naming_elab_this_hint.elab_typedef;
      elab_haccess = Naming_elab_haccess_hint.elab_typedef;
      validate_cast = Naming_validate_cast_expr.validate_typedef;
      elab_collection = Naming_elab_collection.elab_typedef;
      elab_call = Naming_elab_call.elab_typedef;
      elab_tuple = Naming_elab_tuple.elab_typedef;
      elab_invariant = Naming_elab_invariant.elab_typedef;
      elab_const_expr = Naming_elab_const_expr.elab_typedef;
      elab_user_attrs = Naming_elab_user_attributes.elab_typedef;
      elab_import = Naming_elab_import.elab_typedef;
      elab_lvar = Naming_elab_lvar.elab_typedef;
      elab_as_expr = Naming_elab_as_expr.elab_typedef;
      elab_block = Naming_elab_block.elab_typedef;
      elab_pipe = Naming_elab_pipe.elab_typedef;
      elab_func_body = Naming_elab_func_body.elab_typedef;
      elab_lambda_captures = Naming_captures.elab_typedef;
      elab_class_members = Naming_elab_class_members.elab_typedef;
      elab_soft = Naming_elab_soft.elab_typedef;
      elab_everything_sdt = Naming_elab_everything_sdt.elab_typedef;
      elab_hkt = Naming_elab_hkt.elab_typedef;
      elab_enum_class = Naming_elab_enum_class.elab_typedef;
      elab_class_id = Naming_elab_class_id.elab_typedef;
      elab_dynamic_class_name = Naming_elab_dynamic_class_name.elab_typedef;
      validate_fun_params = Naming_validate_fun_params.validate_typedef;
      validate_xhp = Naming_validate_xhp_name.validate_typedef;
      validate_builtin_enum = Naming_validate_builtin_enum.validate_typedef;
      validate_enum_class_typeconst =
        Naming_validate_enum_class_typeconst.validate_typedef;
      validate_supportdyn = Naming_validate_supportdyn.validate_typedef;
      validate_module = Naming_validate_module.validate_typedef;
      validate_class_req = Naming_validate_class_req.validate_typedef;
      validate_consistent_construct =
        Naming_validate_consistent_construct.validate_typedef;
    }

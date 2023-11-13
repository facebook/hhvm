(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env
module MakeType = Typing_make_type

(*****************************************************************************)
(* Construct a Typing_env from an AAST toplevel definition.
 *)
(*****************************************************************************)

open Aast

let fun_env ?origin ctx fd =
  let file = Pos.filename (fst fd.fd_name) in
  let droot = Some (Typing_deps.Dep.Fun (snd fd.fd_name)) in
  let env = Typing_env_types.empty ?origin ctx file ~mode:fd.fd_mode ~droot in
  Typing_inference_env.Identifier_provider.reinitialize ();
  env

let stmt_env ?origin ctx (pos, _) =
  let file = Pos.filename pos in
  Typing_env_types.empty ?origin ctx file ~droot:None

(* Given a class definition construct a type consisting of the
 * class instantiated at its generic parameters. *)
let get_self_from_c c =
  let tparams =
    List.map c.c_tparams ~f:(fun { tp_name = (p, s); _ } ->
        mk
          ( Reason.Rwitness_from_decl (Pos_or_decl.of_raw_pos p),
            Tgeneric (s, []) ))
  in
  let (name_pos, name) = c.c_name in
  let name_pos = Pos_or_decl.of_raw_pos name_pos in
  mk (Reason.Rwitness_from_decl name_pos, Tapply ((name_pos, name), tparams))

(** Set 'self' identifier and type in environment. *)
let set_self env c =
  let self_id = snd c.c_name in
  let self = get_self_from_c c in
  (* For enums, localize makes self:: into an abstract type, which we don't
   * want *)
  let (env, self_ty) =
    match c.c_kind with
    | Ast_defs.Cenum_class _
    | Ast_defs.Cenum ->
      ( env,
        MakeType.class_type (Reason.Rwitness (fst c.c_name)) (snd c.c_name) []
      )
    | Ast_defs.Cinterface
    | Ast_defs.Cclass _
    | Ast_defs.Ctrait ->
      let ((env, ty_err_opt), res) =
        Typing_phase.localize_no_subst env ~ignore_errors:true self
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      (env, res)
  in
  let env = Env.set_self env self_id self_ty in
  let env =
    Env.add_upper_bound env Naming_special_names.Typehints.this self_ty
  in
  let env =
    if c.c_final then
      Env.add_lower_bound env Naming_special_names.Typehints.this self_ty
    else
      env
  in
  env

let set_parent env c =
  (* In order to type-check a class, we need to know what "parent"
   * refers to. Sometimes people write "parent::", when that happens,
   * we need to know the type of parent.
   *)
  match c.c_extends with
  | ((_, Happly ((_, parent_id), _)) as parent_ty) :: _ ->
    let parent_ty = Decl_hint.hint env.Typing_env_types.decl_env parent_ty in
    Env.set_parent env parent_id parent_ty
  (* The only case where we have more than one parent class is when
   * dealing with interfaces and interfaces cannot use parent.
   *)
  | _ :: _
  | _ ->
    env

let class_env ?origin ctx c =
  let file = Pos.filename (fst c.c_name) in
  let droot = Some (Typing_deps.Dep.Type (snd c.c_name)) in
  let env = Typing_env_types.empty ?origin ctx file ~mode:c.c_mode ~droot in
  Typing_inference_env.Identifier_provider.reinitialize ();
  let env = Env.set_current_module env c.c_module in
  let env = set_self env c in
  let env = set_parent env c in
  env

let typedef_env ?origin ctx t =
  let file = Pos.filename (fst t.t_kind) in
  let droot = Some (Typing_deps.Dep.Type (snd t.t_name)) in
  let env = Typing_env_types.empty ?origin ctx file ~mode:t.t_mode ~droot in
  Typing_inference_env.Identifier_provider.reinitialize ();
  env

let gconst_env ?origin ctx cst =
  let file = Pos.filename (fst cst.cst_name) in
  let droot = Some (Typing_deps.Dep.GConst (snd cst.cst_name)) in
  let env = Typing_env_types.empty ?origin ctx file ~mode:cst.cst_mode ~droot in
  Typing_inference_env.Identifier_provider.reinitialize ();
  env

let module_env ?origin ctx md =
  let file = Pos.filename (fst md.md_name) in
  let droot = Some (Typing_deps.Dep.Module (snd md.md_name)) in
  Typing_env_types.empty ?origin ctx file ~mode:md.md_mode ~droot

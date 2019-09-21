(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module Env = Typing_env
module MakeType = Typing_make_type

(*****************************************************************************)
(* Construct a Typing_env from an AAST toplevel definition.
 *)
(*****************************************************************************)

open Aast

let fun_env tcopt f =
  let file = Pos.filename (fst f.f_name) in
  let droot = Some (Typing_deps.Dep.Fun (snd f.f_name)) in
  let env = Env.empty tcopt file ~mode:f.f_mode ~droot in
  env

(* Given a class definition construct a type consisting of the
 * class instantiated at its generic parameters. *)
let get_self_from_c c =
  let tparams =
    List.map c.c_tparams.c_tparam_list (fun { tp_name = (p, s); _ } ->
        (Reason.Rwitness p, Tgeneric s))
  in
  (Reason.Rwitness (fst c.c_name), Tapply (c.c_name, tparams))

let class_env tcopt c =
  let file = Pos.filename (fst c.c_name) in
  let droot = Some (Typing_deps.Dep.Class (snd c.c_name)) in
  let env = Env.empty tcopt file ~mode:c.c_mode ~droot in
  (* Set up self identifier and type *)
  let env = Env.set_self_id env (snd c.c_name) in
  let self = get_self_from_c c in
  (* For enums, localize makes self:: into an abstract type, which we don't
   * want *)
  let (env, self) =
    match c.c_kind with
    | Ast_defs.Cenum -> (env, MakeType.class_type (fst self) (snd c.c_name) [])
    | Ast_defs.Cinterface
    | Ast_defs.Cabstract
    | Ast_defs.Ctrait
    | Ast_defs.Cnormal ->
      Typing_phase.localize_with_self env self
  in
  let env = Env.set_self env self in
  let env =
    match c.c_extends with
    | ((_, Happly ((_, parent_id), _)) as _parent_ty) :: _ ->
      Env.set_parent_id env parent_id
    | _ -> env
  in
  (* Set the ppl env flag *)
  let is_ppl =
    List.exists c.c_user_attributes (fun { ua_name; _ } ->
        SN.UserAttributes.uaProbabilisticModel = snd ua_name)
  in
  let env = Env.set_inside_ppl_class env is_ppl in
  env

let record_def_env tcopt rd =
  let file = Pos.filename (fst rd.rd_name) in
  let droot = Some (Typing_deps.Dep.Class (snd rd.rd_name)) in
  let env = Env.empty tcopt file ~droot in
  env

let typedef_env tcopt t =
  let file = Pos.filename (fst t.t_kind) in
  let droot = Some (Typing_deps.Dep.Class (snd t.t_name)) in
  let env = Env.empty tcopt file ~mode:t.t_mode ~droot in
  env

let gconst_env tcopt cst =
  let file = Pos.filename (fst cst.cst_name) in
  let droot = Some (Typing_deps.Dep.GConst (snd cst.cst_name)) in
  let env = Env.empty tcopt file ~mode:cst.cst_mode ~droot in
  env

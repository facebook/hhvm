(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** {!Tast_env.env} is just an alias to {!Typing_env.env}, and the functions we
    provide for it are largely just aliases to functions that take a
    {!Typing_env.env}.

    If you find that you need to add a new alias here, please take care to
    ensure that it always works with the information available in the {!env}
    constructed by {!Tast_visitor} classes. Only a subset of the information
    available in the inference phase ({!module:Typing}) will be persisted in a
    {!Tast.program} (and thus available to {!Tast_visitor}). Most of the
    persisted information comes from {!Typing_env.save}. *)

let show_env _ = "<env>"
let pp_env _ _ = Printf.printf "%s\n" "<env>"

type env = Typing_env.env
type t = env [@@deriving show]

let print_ty = Typing_print.full_strip_ns
let print_ty_with_identity = Typing_print.full_with_identity
let ty_to_json = Typing_print.to_json

let get_self_id = Typing_env.get_self_id
let get_self = Typing_env.get_self
let is_static = Typing_env.is_static
let is_strict = Typing_env.is_strict
let get_tcopt = Typing_env.get_tcopt
let expand_type = Typing_env.expand_type
let set_static = Typing_env.set_static
let save = Typing_env.save SMap.empty

let fully_expand = Typing_expand.fully_expand

let get_class_ids = Typing_utils.get_class_ids
let fold_unresolved = Typing_utils.fold_unresolved
let push_option_out = Typing_utils.push_option_out

let is_visible = Typing_visibility.is_visible

let hint_to_ty env = Decl_hint.hint env.Typing_env.decl_env

let localize_with_self = Typing_phase.localize_with_self

let referenced_typeconsts env root ids =
  let root = hint_to_ty env root in
  let ety_env = {(Typing_phase.env_with_self env) with
                  Typing_defs.from_class = Some Nast.CIstatic} in
  Typing_taccess.referenced_typeconsts env ety_env (fst root) (root, ids)

let empty tcopt = Typing_env.empty tcopt Relative_path.default ~droot:None

let restore_saved_env env saved_env =
  let module Env = Typing_env in
  {env with
    Env.genv = {env.Env.genv with Env.tcopt = saved_env.Tast.tcopt};
    Env.tenv = IMap.union env.Env.tenv saved_env.Tast.tenv;
    Env.subst = IMap.union env.Env.subst saved_env.Tast.subst;
    Env.global_tpenv = saved_env.Tast.tpenv;
  }

module EnvFromDef = Typing_env_from_def.EnvFromDef(Tast.Annotations)
open Tast

let restore_method_env env m =
  restore_saved_env env m.m_annotation

let restore_fun_env env f =
  restore_saved_env env f.f_annotation

let fun_env f =
  let env = EnvFromDef.fun_env f.f_annotation.tcopt f in
  restore_fun_env env f

let class_env c =
  let env = EnvFromDef.class_env c.c_annotation.tcopt c in
  restore_saved_env env c.c_annotation

let typedef_env t =
  let env = EnvFromDef.typedef_env t.t_annotation.tcopt t in
  restore_saved_env env t.t_annotation

let gconst_env cst =
  let env = EnvFromDef.gconst_env cst.cst_annotation.tcopt cst in
  restore_saved_env env cst.cst_annotation

let def_env d =
  match d with
  | Fun x -> fun_env x
  | Class x -> class_env x
  | Typedef x -> typedef_env x
  | Constant x -> gconst_env x

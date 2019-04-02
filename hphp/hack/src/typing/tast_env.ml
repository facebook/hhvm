(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
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

exception Not_in_class

let print_ty = Typing_print.full_strip_ns

let print_error_ty = Typing_print.error
let print_ty_with_identity = Typing_print.full_with_identity
let ty_to_json = Typing_print.to_json
let json_to_locl_ty = Typing_print.json_to_locl_ty

let get_self_id_exn env =
  match Typing_env.get_self_id env with
  | "" -> raise Not_in_class
  | id -> id

let get_self_id env =
  try Some (get_self_id_exn env) with
  | Not_in_class -> None

let get_self_exn env =
  let _ = get_self_id_exn env in
  Typing_env.get_self env

let get_self env =
  try Some (get_self_exn env) with
  | Not_in_class -> None

let fresh_type = Typing_env.fresh_type
let open_tyvars = Typing_env.open_tyvars
let close_tyvars_and_solve = Typing_subtype.close_tyvars_and_solve
let set_tyvar_variance env ty = Typing_env.set_tyvar_variance env ty
let get_class = Typing_env.get_class
let is_static = Typing_env.is_static
let is_strict = Typing_env.is_strict
let get_tcopt = Typing_env.get_tcopt
let expand_type = Typing_env.expand_type
let set_static = Typing_env.set_static
let set_inside_constructor env = { env with Typing_env.inside_constructor = true }
let get_inside_constructor env = env.Typing_env.inside_constructor
let get_decl_env env = env.Typing_env.decl_env
let get_inside_ppl_class env = env.Typing_env.inside_ppl_class

let get_file = Typing_env.get_file

let fully_expand = Typing_expand.fully_expand

let get_class_ids = Typing_utils.get_class_ids
let fold_unresolved = Typing_utils.fold_unresolved
let flatten_unresolved = Typing_utils.flatten_unresolved
let push_option_out = Typing_utils.push_option_out Pos.none
let get_concrete_supertypes = Typing_utils.get_concrete_supertypes

let is_visible = Typing_visibility.is_visible

let assert_nontrivial = Typing_equality_check.assert_nontrivial
let assert_nullable = Typing_equality_check.assert_nullable

let hint_to_ty env = Decl_hint.hint env.Typing_env.decl_env

let localize_with_self = Typing_phase.localize_with_self
let localize_with_dty_validator = Typing_phase.localize_with_dty_validator

let get_upper_bounds = Typing_env.get_upper_bounds

let is_fresh_generic_parameter = Typing_env.is_fresh_generic_parameter

let simplify_unions env ty = Typing_union.simplify_unions env ty

let is_untyped env ty =
  Typing_utils.is_any env ty || Typing_utils.is_dynamic env ty

let get_reified = Typing_env.get_reified
let get_enforceable = Typing_env.get_enforceable
let get_newable = Typing_env.get_newable

let subtype env ty_sub ty_super =
  Errors.ignore_ (fun () ->
    Errors.try_
      (fun () -> Typing_subtype.sub_type env ty_sub ty_super, true)
      (fun _ -> env, false))

let can_subtype env ty_sub ty_super =
  snd (subtype env ty_sub ty_super)

let is_stringish ?allow_mixed env ty =
  Errors.ignore_ (fun () ->
    Errors.try_
      (fun () -> let _ = Typing_subtype.sub_string ?allow_mixed Pos.none env ty in true)
      (fun _ -> false))

let referenced_typeconsts env root ids =
  let root = hint_to_ty env root in
  let ety_env = {(Typing_phase.env_with_self env) with
                  Typing_defs.from_class = Some Nast.CIstatic} in
  Typing_taccess.referenced_typeconsts env ety_env (fst root) (root, ids)

let empty tcopt = Typing_env.empty tcopt Relative_path.default ~droot:None

let restore_saved_env env saved_env =
  let module Env = Typing_env in
  {env with
    Env.genv = {
      env.Env.genv with
        Env.tcopt = saved_env.Tast.tcopt;
        Env.fun_mutable = saved_env.Tast.fun_mutable};
    Env.tenv = IMap.union env.Env.tenv saved_env.Tast.tenv;
    Env.subst = IMap.union env.Env.subst saved_env.Tast.subst;
    Env.global_tpenv = saved_env.Tast.tpenv;
    Env.lenv = {
      env.Env.lenv with
        Env.local_reactive = saved_env.Tast.reactivity;
        Env.local_mutability = saved_env.Tast.local_mutability};
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
  (* The following nodes are included in the TAST, but are not typechecked.
   * However, we need to return an env here so for now create an empty env using
   * the default typechecker options.
   *)
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | FileAttributes _ -> empty GlobalOptions.default

let set_ppl_lambda env =
  { env with Typing_env.inside_ppl_class = false }

let get_anonymous_lambda_types env id =
  match Typing_env.get_anonymous env id with
  | Some (_, _, ftys, _, _) ->
    let (untyped, typed) = !ftys in
    untyped @ typed
  | _ ->
    []

let typing_env_as_tast_env env = env
let tast_env_as_typing_env env = env

let can_coerce = Typing_ops.can_coerce

let is_xhp_child = Typing_xhp.is_xhp_child

let get_enum = Typing_env.get_enum

let env_reactivity = Typing_env.env_reactivity

let function_is_mutable = Typing_env.function_is_mutable

let local_is_mutable = Typing_env.local_is_mutable

let get_env_mutability = Typing_env.get_env_mutability

let get_fun = Typing_env.get_fun
let set_env_reactive = Typing_env.set_env_reactive

let set_allow_wildcards env = { env with Typing_env.allow_wildcards = true }

let get_allow_wildcards env = env.Typing_env.allow_wildcards

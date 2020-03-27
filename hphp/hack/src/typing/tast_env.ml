(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

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

type env = Typing_env_types.env

type t = env [@@deriving show]

exception Not_in_class

let print_ty = Typing_print.full_strip_ns

let print_decl_ty = Typing_print.full_strip_ns_decl

let print_error_ty = Typing_print.error

let print_ty_with_identity env phase_ty sym_occurrence sym_definition =
  match phase_ty with
  | Typing_defs.DeclTy ty ->
    let (env, ty) = Typing_phase.localize_with_self env ty in
    Typing_print.full_with_identity env ty sym_occurrence sym_definition
  | Typing_defs.LoclTy ty ->
    Typing_print.full_with_identity env ty sym_occurrence sym_definition

let ty_to_json = Typing_print.to_json

let json_to_locl_ty = Typing_print.json_to_locl_ty

let get_self_id = Typing_env.get_self_id

let get_self_ty = Typing_env.get_self_ty

let get_self_ty_exn env =
  match get_self_ty env with
  | Some self_ty -> self_ty
  | None -> raise Not_in_class

let get_class = Typing_env.get_class

let is_static = Typing_env.is_static

let is_strict = Typing_env.is_strict

let get_mode = Typing_env.get_mode

let get_tcopt = Typing_env.get_tcopt

let get_ctx = Typing_env.get_ctx

let expand_type = Typing_env.expand_type

let set_static = Typing_env.set_static

let set_val_kind = Typing_env.set_val_kind

let set_inside_constructor env =
  { env with Typing_env_types.inside_constructor = true }

let get_inside_constructor env = env.Typing_env_types.inside_constructor

let get_decl_env env = env.Typing_env_types.decl_env

let get_inside_ppl_class env = env.Typing_env_types.inside_ppl_class

let get_val_kind = Typing_env.get_val_kind

let get_file = Typing_env.get_file

let fully_expand = Typing_expand.fully_expand

let get_class_ids = Typing_utils.get_class_ids

let non_null = Typing_solver.non_null

let get_concrete_supertypes = Typing_utils.get_concrete_supertypes

let is_visible = Typing_visibility.is_visible

let assert_nontrivial = Typing_equality_check.assert_nontrivial

let assert_nullable = Typing_equality_check.assert_nullable

let hint_to_ty env = Decl_hint.hint env.Typing_env_types.decl_env

let localize env ety_env = Typing_phase.localize ~ety_env env

let localize_with_self = Typing_phase.localize_with_self

let get_upper_bounds = Typing_env.get_upper_bounds

let is_fresh_generic_parameter = Typing_env.is_fresh_generic_parameter

let simplify_unions env ty = Typing_union.simplify_unions env ty

let union_list env r tyl = Typing_union.union_list env r tyl

let get_reified = Typing_env.get_reified

let get_enforceable = Typing_env.get_enforceable

let get_newable = Typing_env.get_newable

let assert_subtype p reason env ty_have ty_expect on_error =
  Typing_ops.sub_type p reason env ty_have ty_expect on_error

let is_sub_type env ty_sub ty_super =
  Typing_subtype.is_sub_type env ty_sub ty_super

let can_subtype env ty_sub ty_super =
  Typing_subtype.can_sub_type env ty_sub ty_super

let is_sub_type_for_union env ty_sub ty_super =
  Typing_subtype.is_sub_type_for_union env ty_sub ty_super

let referenced_typeconsts env root ids =
  let root = hint_to_ty env root in
  let ety_env =
    {
      (Typing_phase.env_with_self env) with
      Typing_defs.from_class = Some CIstatic;
    }
  in
  Typing_taccess.referenced_typeconsts
    env
    ety_env
    (root, ids)
    ~on_error:Errors.unify_error

let empty ctx = Typing_env.empty ctx Relative_path.default ~droot:None

let restore_saved_env env saved_env =
  let module Env = Typing_env_types in
  let ctx =
    Provider_context.map_tcopt env.Env.decl_env.Decl_env.ctx ~f:(fun _tcopt ->
        saved_env.Tast.tcopt)
  in
  let decl_env = { env.Env.decl_env with Decl_env.ctx } in
  {
    env with
    Env.decl_env;
    Env.pessimize = saved_env.Tast.pessimize;
    Env.genv =
      {
        env.Env.genv with
        Env.tcopt = saved_env.Tast.tcopt;
        Env.fun_mutable = saved_env.Tast.fun_mutable;
        Env.condition_types = saved_env.Tast.condition_types;
      };
    Env.inference_env =
      Typing_inference_env.simple_merge
        env.Env.inference_env
        saved_env.Tast.inference_env;
    Env.global_tpenv = saved_env.Tast.tpenv;
    Env.lenv =
      {
        env.Env.lenv with
        Env.local_reactive = saved_env.Tast.reactivity;
        Env.local_mutability = saved_env.Tast.local_mutability;
      };
  }

module EnvFromDef = Typing_env_from_def
open Tast

let restore_method_env env m = restore_saved_env env m.m_annotation

let restore_fun_env env f = restore_saved_env env f.f_annotation

let restore_pu_enum_env env pu = restore_saved_env env pu.pu_annotation

let fun_env ctx f =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> f.f_annotation.tcopt)
  in
  let env = EnvFromDef.fun_env ctx f in
  restore_fun_env env f

let class_env ctx c =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> c.c_annotation.tcopt)
  in
  let env = EnvFromDef.class_env ctx c in
  restore_saved_env env c.c_annotation

let typedef_env ctx t =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> t.t_annotation.tcopt)
  in
  let env = EnvFromDef.typedef_env ctx t in
  restore_saved_env env t.t_annotation

let gconst_env ctx cst =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> cst.cst_annotation.tcopt)
  in
  let env = EnvFromDef.gconst_env ctx cst in
  restore_saved_env env cst.cst_annotation

let def_env ctx d =
  match d with
  | Fun x -> fun_env ctx x
  | Class x -> class_env ctx x
  | Typedef x -> typedef_env ctx x
  | Constant x -> gconst_env ctx x
  | RecordDef _ -> empty ctx
  (* TODO T44306013 *)
  (* The following nodes are included in the TAST, but are not typechecked.
   * However, we need to return an env here so for now create an empty env using
   * the default typechecker options.
   *)
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | FileAttributes _ ->
    empty ctx

let set_ppl_lambda env = { env with Typing_env_types.inside_ppl_class = false }

let typing_env_as_tast_env env = env

let tast_env_as_typing_env env = env

let is_xhp_child = Typing_xhp.is_xhp_child

let get_enum = Typing_env.get_enum

let is_typedef = Typing_env.is_typedef

let get_typedef = Typing_env.get_typedef

let is_enum = Typing_env.is_enum

let env_reactivity = Typing_env_types.env_reactivity

let function_is_mutable = Typing_env.function_is_mutable

let local_is_mutable = Typing_env.local_is_mutable

let get_env_mutability = Typing_env.get_env_mutability

let get_fun = Typing_env.get_fun

let set_env_reactive = Typing_env.set_env_reactive

let set_allow_wildcards env =
  { env with Typing_env_types.allow_wildcards = true }

let get_allow_wildcards env = env.Typing_env_types.allow_wildcards

let condition_type_matches = Typing_reactivity.condition_type_matches

(* ocaml being ocaml...
 * We need at least one explicit reference to the Typing_pocket_univereses
 * module otherwise the compiler will not include it in the resulting binary.
 * Because of cyclic module dependencies, this is never done directly (we
 * rely on references in Typing_utils), so I need this dummy occurence just
 * to make sure the code is present.
 *)
let _ =
  let _ = Typing_pocket_universes.expand_dep_ty in
  ()

(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

(* Eliminate residue of type inference:
 *   1. Tvars are replaced (deep) by the expanded type
 *   2. Singleton unions are eliminated
 * TODO TAST:
 *   Transform completely unconstrained types to Tmixed
 *   Consider using a fresh datatype for TAST types.
 *)
let expand_ty ?var_hook ?pos env ty =
  let rec exp_ty ty =
    begin
      match (get_node ty, var_hook) with
      | (Tvar var, Some hook) -> hook var
      | _ -> ()
    end;
    let (_, ety) = Tast_env.expand_type env ty in
    let ety =
      match deref ety with
      | (_, (Tany _ | Tnonnull | Tprim _ | Tdynamic | Tneg _)) -> ety
      | (p, Tgeneric (name, args)) -> mk (p, Tgeneric (name, exp_tys args))
      | (p, Tclass (n, e, tyl)) -> mk (p, Tclass (n, e, exp_tys tyl))
      | (p, Tunion tyl) -> mk (p, Tunion (exp_tys tyl))
      | (p, Tintersection tyl) -> mk (p, Tintersection (exp_tys tyl))
      | (p, Toption ty) -> mk (p, Toption (exp_ty ty))
      | (p, Ttuple tyl) -> mk (p, Ttuple (exp_tys tyl))
      | (p, Tfun ft) -> mk (p, Tfun (exp_fun_type ft))
      | (p, Tnewtype (n, tyl, ty)) ->
        mk (p, Tnewtype (n, exp_tys tyl, exp_ty ty))
      | (p, Tdependent (n, ty)) -> mk (p, Tdependent (n, exp_ty ty))
      | (p, Tshape s) -> mk (p, Tshape (exp_shape_type s))
      | (p, Tvec_or_dict (ty1, ty2)) ->
        mk (p, Tvec_or_dict (exp_ty ty1, exp_ty ty2))
      | (p, Tvar v) ->
        (match pos with
        | None -> mk (p, Tvar v)
        | Some pos ->
          if
            TypecheckerOptions.disallow_unresolved_type_variables
              (Tast_env.get_tcopt env)
          then
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(primary @@ Primary.Unresolved_tyvar pos);
          mk (p, Tvar v))
      (* TODO(T36532263) see if that needs updating *)
      | (_, Taccess _) -> ety
      | (_, Tunapplied_alias _) -> ety
    in
    ety
  and exp_tys tyl = List.map ~f:exp_ty tyl
  and exp_fun_type
      {
        ft_tparams;
        ft_where_constraints;
        ft_ret;
        ft_flags;
        ft_params;
        ft_implicit_params;
        ft_cross_package;
      } =
    {
      ft_flags;
      ft_cross_package;
      ft_tparams = List.map ~f:exp_tparam ft_tparams;
      ft_where_constraints =
        List.map ~f:exp_where_constraint ft_where_constraints;
      ft_ret = exp_ty ft_ret;
      ft_params = List.map ~f:exp_fun_param ft_params;
      ft_implicit_params = exp_fun_implicit_params ft_implicit_params;
    }
  and exp_fun_param { fp_pos; fp_name; fp_type; fp_flags; fp_def_value } =
    { fp_pos; fp_name; fp_type = exp_ty fp_type; fp_flags; fp_def_value }
  and exp_fun_implicit_params { capability } =
    let capability =
      match capability with
      | CapTy ty -> CapTy (exp_ty ty)
      | CapDefaults p -> CapDefaults p
    in
    { capability }
  and exp_sft { sft_optional; sft_ty } =
    { sft_optional; sft_ty = exp_ty sft_ty }
  and exp_shape_type
      {
        s_origin = shape_origin;
        s_unknown_value = shape_kind;
        s_fields = fields;
      } =
    {
      s_origin = shape_origin;
      s_unknown_value = shape_kind;
      (* TODO(shapes) exp_ty s_unknown_value *)
      s_fields = TShapeMap.map exp_sft fields;
    }
  and exp_tparam t =
    {
      t with
      tp_constraints =
        List.map ~f:(fun (ck, ty) -> (ck, exp_ty ty)) t.tp_constraints;
    }
  and exp_where_constraint (ty1, ck, ty2) = (exp_ty ty1, ck, exp_ty ty2) in
  exp_ty ty

let expander =
  object (self)
    inherit Tast_visitor.endo

    method! on_expr env (ty, pos, expr_) =
      (expand_ty ~pos env ty, pos, self#on_expr_ env expr_)

    method! on_class_id env (ty, pos, cid_) =
      (expand_ty ~pos env ty, pos, self#on_class_id_ env cid_)

    method! on_'ex env ty = expand_ty env ty
  end

(* Replace all types in a program AST by their expansions *)
let expand_program ctx tast = expander#go ctx tast

let expand_def ctx def = expander#go_def ctx def

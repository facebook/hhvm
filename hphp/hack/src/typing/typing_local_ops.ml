(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs_core
open Typing_env_types
module Env = Typing_env
module SN = Naming_special_names

let check_local_capability (mk_required : env -> env * locl_ty) mk_err_opt env =
  (* gate the check behavior on coeffects TC option *)
  let tcopt = Env.get_tcopt env in
  let should_skip_check =
    (not @@ TypecheckerOptions.local_coeffects tcopt)
    || TypecheckerOptions.enable_sound_dynamic tcopt
       && Tast.is_under_dynamic_assumptions env.checked
  in
  if not should_skip_check then (
    let available = Env.get_local env Typing_coeffects.local_capability_id in
    let (env, required) = mk_required env in
    let err_opt = mk_err_opt available required in
    let (env, ty_err_opt) =
      Typing_subtype.sub_type_or_fail
        env
        available.Typing_local_types.ty
        required
        err_opt
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    env
  ) else
    env

let enforce_local_capability
    ?((* Run-time enforced ops must have the default as it's unfixmeable *)
      err_code = Error_codes.Typing.OpCoeffects)
    ?suggestion
    (mk_required : env -> env * locl_ty)
    (op : string)
    (op_pos : Pos.t)
    env =
  check_local_capability
    mk_required
    (fun available required ->
      Some
        Typing_error.(
          coeffect
          @@ Primary.Coeffect.Op_coeffect_error
               {
                 pos = op_pos;
                 op_name = op;
                 locally_available =
                   Typing_coeffects.pretty env available.Typing_local_types.ty;
                 available_pos =
                   Typing_defs.get_pos available.Typing_local_types.ty;
                 required = Typing_coeffects.pretty env required;
                 err_code;
                 suggestion;
               }))
    env

module Capabilities = struct
  module Reason = Typing_reason
  include SN.Capabilities

  let mk special_name env =
    let r = Reason.Rnone in
    let ((env, ty_err_opt), res) =
      Typing_make_type.apply r (Reason.to_pos r, special_name) []
      |> Typing_phase.localize_no_subst ~ignore_errors:true env
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    (env, res)
end

let enforce_memoize_object pos env =
  (* Allow zoned_shallow/local policies to memoize objects,
     since those will convert to PolicyShardedMemoize after conversion to zoned *)
  (* We use ImplicitPolicy instead of ImplicitPolicyShallow just to not error
     for zoned, since memoizing a zoned function already has a different error
     associated with it. *)
  let (env, zoned) = Capabilities.(mk implicitPolicy) env in
  let (env, access_globals) = Capabilities.(mk accessGlobals) env in

  let mk_zoned_or_access_globals env =
    let r = Reason.Rnone in
    (env, Typing_make_type.union r [zoned; access_globals])
  in
  check_local_capability
    mk_zoned_or_access_globals
    (fun available _required ->
      Some
        Typing_error.(
          coeffect
          @@ Primary.Coeffect.Op_coeffect_error
               {
                 pos;
                 op_name = "Memoizing object parameters";
                 locally_available =
                   Typing_coeffects.pretty env available.Typing_local_types.ty;
                 available_pos =
                   Typing_defs.get_pos available.Typing_local_types.ty;
                 (* Use access globals in error message *)
                 required = Typing_coeffects.pretty env access_globals;
                 (* Temporarily FIXMEable error for memoizing objects. Once ~65 current cases are removed
                    we can change this *)
                 err_code = Error_codes.Typing.MemoizeObjectWithoutGlobals;
                 suggestion = None;
               }))
    env

let enforce_io =
  enforce_local_capability Capabilities.(mk io) "`echo` or `print` builtin"

let enforce_enum_class_variant =
  enforce_local_capability
    Capabilities.(mk writeProperty)
    "Accessing an enum class constant"

(* basic local mutability checks *)
let rec is_byval_collection_or_string_or_any_type env ty =
  let check ty =
    let open Aast in
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Toption inner -> is_byval_collection_or_string_or_any_type env inner
    | Tclass ((_, x), _, _) ->
      String.equal x SN.Collections.cVec
      || String.equal x SN.Collections.cDict
      || String.equal x SN.Collections.cKeyset
    | Tvec_or_dict _
    | Ttuple _
    | Tshape _
    | Tprim Tstring
    | Tdynamic
    | Tany _ ->
      true
    | Tunion tyl ->
      List.for_all tyl ~f:(is_byval_collection_or_string_or_any_type env)
    | Tintersection tyl ->
      List.exists tyl ~f:(is_byval_collection_or_string_or_any_type env)
    | Tgeneric _
    | Tnewtype _
    | Tdependent _ ->
      (* FIXME we should probably look at the upper bounds here. *)
      false
    | Tnonnull
    | Tprim _
    | Tfun _
    | Tvar _
    | Taccess _
    | Tneg _ ->
      false
    | Tunapplied_alias _ ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
  in
  let (_, tl) =
    Typing_utils.get_concrete_supertypes ~abstract_enum:true env ty
  in
  List.for_all tl ~f:check

let mutating_this_in_ctor env (_, _, e) : bool =
  match e with
  | Aast.This when Env.fun_is_constructor env -> true
  | _ -> false

let rec is_valid_mutable_subscript_expression_target env v =
  let open Aast in
  match v with
  | (_, _, Hole (e, _, _, _)) ->
    is_valid_mutable_subscript_expression_target env e
  | (ty, _, Lvar _) -> is_byval_collection_or_string_or_any_type env ty
  | (ty, _, Array_get (e, _)) ->
    is_byval_collection_or_string_or_any_type env ty
    && is_valid_mutable_subscript_expression_target env e
  | (ty, _, Obj_get (e, _, _, _)) ->
    is_byval_collection_or_string_or_any_type env ty
    && (is_valid_mutable_subscript_expression_target env e
       || mutating_this_in_ctor env e)
  | (_, _, ReadonlyExpr e) -> is_valid_mutable_subscript_expression_target env e
  | _ -> false

let is_valid_append_target _env ty =
  (* TODO(coeffects) move this to typing.ml and completely redesign
     the line below (as it was in Tast_check) doesn't do the right thing *)
  (* let (_env, ty) = Env.expand_type _env ty in *)
  match get_node ty with
  | Tclass ((_, n), _, []) ->
    String.( <> ) n SN.Collections.cVector
    && String.( <> ) n SN.Collections.cSet
    && String.( <> ) n SN.Collections.cMap
  | Tclass ((_, n), _, [_]) ->
    String.( <> ) n SN.Collections.cVector
    && String.( <> ) n SN.Collections.cSet
  | Tclass ((_, n), _, [_; _]) -> String.( <> ) n SN.Collections.cMap
  | _ -> true

let rec check_assignment_or_unset_target
    ~is_assignment
    ?append_pos_opt
    env
    te1
    capability_available
    capability_required =
  let fail ?suggestion op_name pos =
    Some
      Typing_error.(
        coeffect
        @@ Primary.Coeffect.Op_coeffect_error
             {
               pos;
               op_name;
               suggestion;
               locally_available =
                 Typing_coeffects.pretty
                   env
                   capability_available.Typing_local_types.ty;
               available_pos =
                 Typing_defs.get_pos capability_available.Typing_local_types.ty;
               required = Typing_coeffects.pretty env capability_required;
               err_code = Error_codes.Typing.OpCoeffects;
             })
  in
  let (_, p, expr_) = te1 in
  let open Aast in
  match expr_ with
  | Hole (e, _, _, _) ->
    check_assignment_or_unset_target
      ~is_assignment
      ?append_pos_opt
      env
      e
      capability_available
      capability_required
  | Obj_get (e1, _, _, _) when mutating_this_in_ctor env e1 -> None
  | Array_get ((ty1, _, _), i)
    when is_assignment && not (is_valid_append_target env ty1) ->
    let is_append = Option.is_none i in
    let msg_prefix =
      if is_append then
        "Appending to a Hack Collection object"
      else
        "Assigning to an element of a Hack Collection object via `[]`"
    in
    fail msg_prefix (Option.value append_pos_opt ~default:p)
  | Array_get (e1, _) when is_valid_mutable_subscript_expression_target env e1
    ->
    None
  (* we already report errors about statics in rx context, no need to do it twice *)
  | Class_get _ -> None
  | Obj_get _
  | Array_get _ ->
    if is_assignment then
      fail
        ("This object's property is being mutated (used as an lvalue)"
        ^ "\nSetting non-mutable object properties")
        p
    else
      fail "Non-mutable argument for `unset`" p
  | _ -> None

(* END logic from Typed AST check in basic_reactivity_check *)

let rec check_assignment env (x, append_pos_opt, te_) =
  let open Ast_defs in
  match te_ with
  | Aast.Hole ((_, _, e), _, _, _) -> check_assignment env (x, append_pos_opt, e)
  | Aast.Unop ((Uincr | Udecr | Upincr | Updecr), te1)
  | Aast.(Binop { bop = Eq _; lhs = te1; _ }) ->
    check_local_capability
      Capabilities.(mk writeProperty)
      (check_assignment_or_unset_target
         ~is_assignment:true
         ~append_pos_opt
         env
         te1)
      env
  | _ -> env

let check_unset_target env (tel : (Ast_defs.param_kind * Tast.expr) list) =
  List.fold ~init:env tel ~f:(fun env (_, te) ->
      check_local_capability
        Capabilities.(mk writeProperty)
        (fun available required ->
          check_assignment_or_unset_target
            ~is_assignment:false
            env
            te
            available
            required)
        env)

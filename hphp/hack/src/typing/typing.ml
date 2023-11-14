(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This module implements the typing.
 *
 * Given an Nast.program, it infers the type of all the local
 * variables, and checks that all the types are correct (aka
 * consistent) *)
open Hh_prelude
open Common
open Aast
open Tast
open Typing_defs
open Typing_env_types
open Utils
open Typing_helpers
module TFTerm = Typing_func_terminality
module TUtils = Typing_utils
module Reason = Typing_reason
module Type = Typing_ops
module Env = Typing_env
module Inf = Typing_inference_env
module LEnv = Typing_lenv
module Async = Typing_async
module SubType = Typing_subtype
module Union = Typing_union
module Inter = Typing_intersection
module SN = Naming_special_names
module TVis = Typing_visibility
module Phase = Typing_phase
module TOG = Typing_object_get
module Subst = Decl_subst
module ExprDepTy = Typing_dependent_type.ExprDepTy
module TCO = TypecheckerOptions
module C = Typing_continuations
module CMap = C.Map
module Try = Typing_try
module FL = FeatureLogging
module MakeType = Typing_make_type
module Cls = Decl_provider.Class
module Fake = Typing_fake_members
module ExpectedTy = Typing_helpers.ExpectedTy
module ITySet = Internal_type_set

type newable_class_info =
  env
  * Tast.targ list
  * Tast.class_id
  * [ `Class of pos_id * Cls.t * locl_ty | `Dynamic ] list

type dyn_func_kind =
  | Supportdyn_function
  | Like_function

type branch_info = {
  lset: Local_id.Set.t;
  pkgs: SSet.t;
}

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

(* A guess as to the last position we were typechecking, for use in debugging,
 * such as figuring out what a runaway hh_server thread is doing. Updated
 * only best-effort -- it's an approximation to point debugging in the right
 * direction, nothing more. *)
let debug_last_pos = ref Pos.none

let debug_print_last_pos _ =
  Hh_logger.info
    "Last typecheck pos: %s"
    (Pos.string (Pos.to_absolute !debug_last_pos))

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let log_iteration_count env pos n =
  let should_log_iteration_count =
    Env.get_tcopt env
    |> TCO.log_levels
    |> SMap.find_opt "loop_iteration_count"
    |> Option.is_some
  in
  if should_log_iteration_count then
    let msg =
      Format.asprintf
        "Loop iteration count %s:%a: %d\n"
        (Pos.to_absolute pos |> Pos.filename)
        Pos.pp
        pos
        n
    in
    Hh_logger.log "%s" msg

let mk_ty_mismatch_opt ty_have ty_expect = function
  | Some _ -> Some (ty_have, ty_expect)
  | _ -> None

let mk_ty_mismatch_res ty_have ty_expect = function
  | Some _ -> Error (ty_have, ty_expect)
  | _ -> Ok ty_have

let mk_hole ?(source = Aast.Typing) ((_, pos, _) as expr) ~ty_have ~ty_expect =
  if equal_locl_ty ty_have ty_expect then
    expr
  else
    (* if the hole is generated from typing, we leave the type unchanged,
       if it is a call to `[unsafe|enforced]_cast`, we give it the expected type
    *)
    let ty_hole =
      match source with
      | Aast.Typing -> ty_have
      | UnsafeCast _
      | UnsafeNonnullCast
      | EnforcedCast _ ->
        ty_expect
    in
    make_typed_expr pos ty_hole @@ Aast.Hole (expr, ty_have, ty_expect, source)

let hole_on_ty_mismatch (te : Tast.expr) ~ty_mismatch_opt : Tast.expr =
  Option.value_map ty_mismatch_opt ~default:te ~f:(fun (ty_have, ty_expect) ->
      mk_hole te ~ty_have ~ty_expect)

(* When recording subtyping or coercion errors for union and intersection types
   we need to look at the error for each element and then reconstruct any
   errors into a union or intersection. If there were no errors for any
   element, the result if also `Ok`; if there was an error for at least
   on element we have `Error` with list of actual and expected types *)
let fold_coercion_errs errs =
  List.fold_left errs ~init:(Ok []) ~f:(fun acc err ->
      match (acc, err) with
      | (Ok xs, Ok x) -> Ok (x :: xs)
      | (Ok xs, Error (x, y)) -> Error (x :: xs, y :: xs)
      | (Error (xs, ys), Ok x) -> Error (x :: xs, x :: ys)
      | (Error (xs, ys), Error (x, y)) -> Error (x :: xs, y :: ys))

let union_coercion_errs errs =
  Result.fold
    ~ok:(fun tys -> Ok (MakeType.union Reason.Rnone tys))
    ~error:(fun (acts, exps) ->
      Error (MakeType.union Reason.Rnone acts, MakeType.union Reason.Rnone exps))
  @@ fold_coercion_errs errs

let intersect_coercion_errs errs =
  Result.fold
    ~ok:(fun tys -> Ok (MakeType.intersection Reason.Rnone tys))
    ~error:(fun (acts, exps) ->
      Error
        ( MakeType.intersection Reason.Rnone acts,
          MakeType.intersection Reason.Rnone exps ))
  @@ fold_coercion_errs errs

(** Given the type of an argument that has been unpacked and typed against
    positional and variadic function parameter types, apply the subtyping /
    coercion errors back to the original packed type. *)
let pack_errs pos ty subtyping_errs =
  let nothing =
    MakeType.nothing @@ Reason.Rsolve_fail (Pos_or_decl.of_raw_pos pos)
  in
  let rec aux ~k = function
    (* Case 1: we have a type error at this positional parameter so
       replace the type parameter which caused it with the expected type *)
    | ((Some (_, ty) :: rest, var_opt), _ :: tys)
    (* Case 2: there was no type error here so retain the original type
       parameter *)
    | ((None :: rest, var_opt), ty :: tys) ->
      (* recurse for any remaining positional parameters and add the
         corrected (case 1) or original (case 2) type to the front of the
         list of type parameters in the continuation *)
      aux ((rest, var_opt), tys) ~k:(fun tys -> k (ty :: tys))
    (* Case 3: we have a type error at the variadic parameter so replace
       the type parameter which cased it with the expected type *)
    | ((_, (Some (_, ty) as var_opt)), _ :: tys) ->
      (* recurse with the variadic parameter error and add the
         corrected type to the front of the list of type parameters in the
         continuation *)
      aux (([], var_opt), tys) ~k:(fun tys -> k (ty :: tys))
    (* Case 4: we have a variadic parameter but no error - we're done so
       pass the remaining unchanged type parameters into the contination
       to rebuild corrected type params in the right order *)
    | ((_, None), tys) -> k tys
    (* Case 5: no more type parameters - again we're done so pass empty
       list to continuation and rebuild corrected type params in the right
       order *)
    | (_, []) -> k []
  in
  (* The only types that _can_ be upacked are tuples and pairs; match on the
     type to get the type parameters, pass them to our recursive function
     aux to subsitute the expected type where we have a type error
     then reconstruct the type in the continuation *)
  match deref ty with
  | (r, Ttuple tys) ->
    aux (subtyping_errs, tys) ~k:(fun tys -> mk (r, Ttuple tys))
  | (r, Tclass (pos_id, exact, tys)) ->
    aux (subtyping_errs, tys) ~k:(fun tys ->
        mk (r, Tclass (pos_id, exact, tys)))
  | _ -> nothing

let triple_to_pair (env, te, ty) = (env, (te, ty))

let with_special_coeffects env cap_ty unsafe_cap_ty f =
  let init =
    Option.map (Env.next_cont_opt env) ~f:(fun next_cont ->
        let initial_locals = next_cont.Typing_per_cont_env.local_types in
        let tpenv = Env.get_tpenv env in
        (initial_locals, tpenv))
  in
  LEnv.stash_and_do env (Env.all_continuations env) (fun env ->
      let env =
        match init with
        | None -> env
        | Some (initial_locals, tpenv) ->
          let env = Env.reinitialize_locals env in
          let env = Env.set_locals env initial_locals in
          let env = Env.env_with_tpenv env tpenv in
          env
      in
      let (env, _ty) =
        Typing_coeffects.register_capabilities env cap_ty unsafe_cap_ty
      in
      f env)

(* Set all the types in an expression to the given type. *)
let with_type ty env (e : Nast.expr) : Tast.expr =
  let visitor =
    object (self)
      inherit [_] Aast.map

      method! on_expr env ((), p, expr_) = (ty, p, self#on_expr_ env expr_)

      method on_'ex _ () = ty

      method on_'en _ _ = env
    end
  in
  visitor#on_expr () e

let expr_error env p (e : Nast.expr) =
  let (env, ty) = Env.fresh_type_error env p in
  (env, with_type ty Tast.dummy_saved_env e, ty)

let unbound_name env (pos, name) e =
  if Tast.is_under_dynamic_assumptions env.checked then
    expr_error env pos e
  else
    let class_exists =
      let ctx = Env.get_ctx env in
      let decl = Decl_provider.get_class ctx name in
      match decl with
      | None -> false
      | Some dc -> Ast_defs.is_c_class (Cls.kind dc)
    in
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(primary @@ Primary.Unbound_name { pos; name; class_exists });
    expr_error env pos e

(* Is this type Traversable<vty> or Container<vty> for some vty? *)
let get_value_collection_inst env p vc_kind ty =
  let arraykey_on ty =
    match vc_kind with
    | Set
    | ImmSet
    | Keyset ->
      let reason = Reason.Rset_element p in
      let arraykey = MakeType.arraykey reason in
      let (env, ty) = Inter.intersect env ~r:reason arraykey ty in
      Some (env, ty)
    | Vector
    | ImmVector
    | Vec ->
      Some (env, ty)
  in
  match get_node ty with
  | Tclass ((_, c), _, [vty])
    when String.equal c SN.Collections.cTraversable
         || String.equal c SN.Collections.cContainer ->
    arraykey_on vty
  (* If we're expecting a mixed or a nonnull then we can just assume
   * that the element type is mixed if it is a vec-like type and arraykey if it
   * is a set-like one. *)
  | Tnonnull -> arraykey_on (MakeType.mixed (get_reason ty))
  | Tany _ -> Some (env, ty)
  | Tdynamic when Tast.is_under_dynamic_assumptions env.checked ->
    (* interpret dynamic as Traversable<dynamic> or keyset<arraykey> *)
    arraykey_on ty
  | _ -> None

(* Is this type KeyedTraversable<kty,vty>
 *           or KeyedContainer<kty,vty>
 * for some kty, vty?
 *)
let get_key_value_collection_inst env p ty =
  match get_node ty with
  | Tclass ((_, c), _, [kty; vty])
    when String.equal c SN.Collections.cKeyedTraversable
         || String.equal c SN.Collections.cKeyedContainer ->
    let reason = Reason.Rkey_value_collection_key p in
    let arraykey = MakeType.arraykey reason in
    let (env, kty) = Inter.intersect env ~r:reason kty arraykey in
    Some (env, kty, vty)
  (* If we're expecting a mixed or a nonnull then we can just assume
   * that the key type is arraykey and the value type is mixed *)
  | Tnonnull ->
    let arraykey = MakeType.arraykey (Reason.Rkey_value_collection_key p) in
    let mixed = MakeType.mixed (Reason.Rwitness p) in
    Some (env, arraykey, mixed)
  | Tany _ -> Some (env, ty, ty)
  | Tdynamic when Tast.is_under_dynamic_assumptions env.checked ->
    (* interpret dynamic as KeyedTraversable<arraykey, dynamic> *)
    let arraykey = MakeType.arraykey (Reason.Rkey_value_collection_key p) in
    Some (env, arraykey, ty)
  | _ -> None

(* Is this type varray<vty> or a supertype for some vty? *)
let vc_kind_to_supers kind =
  match kind with
  | Vector -> [SN.Collections.cVector; SN.Collections.cMutableVector]
  | ImmVector -> [SN.Collections.cImmVector; SN.Collections.cConstVector]
  | Vec -> [SN.Collections.cVec]
  | Set -> [SN.Collections.cSet; SN.Collections.cMutableSet]
  | ImmSet -> [SN.Collections.cImmSet; SN.Collections.cConstSet]
  | Keyset -> [SN.Collections.cKeyset]

let kvc_kind_to_supers kind =
  match kind with
  | Map -> [SN.Collections.cMap; SN.Collections.cMutableMap]
  | ImmMap -> [SN.Collections.cImmMap; SN.Collections.cConstMap]
  | Dict -> [SN.Collections.cDict]

(* Is this type one of the value collection types with element type vty? *)
let get_vc_inst env p vc_kind ty =
  let classnames = vc_kind_to_supers vc_kind in
  match get_node ty with
  | Tclass ((_, c), _, [vty]) when List.exists classnames ~f:(String.equal c) ->
    Some (env, vty)
  | _ -> get_value_collection_inst env p vc_kind ty

(* Is this type one of the three key-value collection types
 * e.g. dict<kty,vty> or a supertype for some kty and vty? *)
let get_kvc_inst env p kvc_kind ty =
  let classnames = kvc_kind_to_supers kvc_kind in
  match get_node ty with
  | Tclass ((_, c), _, [kty; vty])
    when List.exists classnames ~f:(String.equal c) ->
    Some (env, kty, vty)
  | _ -> get_key_value_collection_inst env p ty

(* Check whether this is a function type that (a) either returns a disposable
 * or (b) has the <<__ReturnDisposable>> attribute
 *)
let is_return_disposable_fun_type env ty =
  let (_env, ty) = Env.expand_type env ty in
  let ty = TUtils.strip_dynamic env ty in
  let (_, _env, ty) = TUtils.strip_supportdyn env ty in
  match get_node ty with
  | Tfun ft ->
    get_ft_return_disposable ft
    || Option.is_some
         (Typing_disposable.is_disposable_type env ft.ft_ret.et_type)
  | _ -> false

(* Turn an environment into a local_id_map suitable to be embedded
 * into an AssertEnv statement
 *)
let annot_map env =
  match Env.next_cont_opt env with
  | Some { Typing_per_cont_env.local_types; _ } ->
    Some
      (Local_id.Map.filter_map
         (fun _
              Typing_local_types.
                { ty; defined; bound_ty = _; pos; eid = _expr_id } ->
           if defined then
             Some (pos, ty)
           else
             None)
         local_types)
  | None -> None

(* Similar to annot_map above, but filters the map to only contain
 * information about locals in lset
 *)
let refinement_annot_map env lset =
  match annot_map env with
  | Some map ->
    let map =
      Local_id.Map.filter (fun lid _ -> Local_id.Set.mem lid lset) map
    in
    if Local_id.Map.is_empty map then
      None
    else
      Some map
  | None -> None

let assert_env_blk ~pos ~at annotation_kind env_map_opt blk =
  let mk_assert map = (pos, Aast.AssertEnv (annotation_kind, map)) in
  let annot_blk = Option.to_list (Option.map ~f:mk_assert env_map_opt) in
  match at with
  | `Start -> annot_blk @ blk
  | `End -> blk @ annot_blk

let assert_env_stmt ~pos ~at annotation_kind env_map_opt stmt =
  let mk_assert map = (pos, Aast.AssertEnv (annotation_kind, map)) in
  match env_map_opt with
  | Some env_map ->
    let stmt = (pos, stmt) in
    let blk =
      match at with
      | `Start -> [mk_assert env_map; stmt]
      | `End -> [stmt; mk_assert env_map]
    in
    Aast.Block (None, blk)
  | None -> stmt

let set_tcopt_unstable_features env { fa_user_attributes; _ } =
  match
    Naming_attributes.find
      SN.UserAttributes.uaEnableUnstableFeatures
      fa_user_attributes
  with
  | None -> env
  | Some { ua_name = _; ua_params } ->
    let ( = ) = String.equal in
    List.fold ua_params ~init:env ~f:(fun env (_, _, feature) ->
        match feature with
        | Aast.String s when s = SN.UnstableFeatures.modules ->
          Env.map_tcopt ~f:(fun t -> TCO.set_modules t true) env
        | Aast.String s when s = SN.UnstableFeatures.expression_trees ->
          Env.map_tcopt
            ~f:(fun t -> TCO.set_tco_enable_expression_trees t true)
            env
        | _ -> env)

(** Do a subtype check of inferred type against expected type.
   * The optional coerce_for_op parameter controls whether any arguments of type
   * dynamic can be coerced to enforceable types because they are arguments to a
   * built-in operator.
 *)
let check_expected_ty_res
    ~(coerce_for_op : bool)
    (message : string)
    (env : env)
    (inferred_ty : locl_ty)
    (expected : ExpectedTy.t option) : (env, env) result =
  match expected with
  | None -> Ok env
  | Some ExpectedTy.{ pos = p; reason = ur; ty; coerce } ->
    Typing_log.(
      log_with_level env "typing" ~level:1 (fun () ->
          log_types
            (Pos_or_decl.of_raw_pos p)
            env
            [
              Log_head
                ( Printf.sprintf
                    "Typing.check_expected_ty %s enforced=%s"
                    message
                    (match ty.et_enforced with
                    | Unenforced -> "unenforced"
                    | Enforced -> "enforced"),
                  [
                    Log_type ("inferred_ty", inferred_ty);
                    Log_type ("expected_ty", ty.et_type);
                  ] );
            ]));
    let (env, ty_err_opt) =
      Typing_coercion.coerce_type
        ~coerce_for_op
        ~coerce
        p
        ur
        env
        inferred_ty
        ty
        Typing_error.Callback.unify_error
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    Option.value_map ~default:(Ok env) ~f:(fun _ -> Error env) ty_err_opt

let check_expected_ty message env inferred_ty expected =
  Result.fold ~ok:Fn.id ~error:Fn.id
  @@ check_expected_ty_res ~coerce_for_op:false message env inferred_ty expected

(* Set a local; must not be already assigned if it is a using variable *)
let set_local ?(is_using_clause = false) ~is_defined ~bound_ty env (pos, x) ty =
  if Env.is_using_var env x then
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
          (if is_using_clause then
            Primary.Duplicate_using_var pos
          else
            Primary.Illegal_disposable { pos; verb = `assigned }));
  let env = Env.set_local ~is_defined ~bound_ty env x ty pos in
  if is_using_clause then
    Env.set_using_var env x
  else
    env

(* Require a new construct with disposable *)
let rec enforce_return_disposable env e =
  match e with
  | (_, _, New _) -> ()
  | (_, _, Call _) -> ()
  | (_, _, Await (_, _, Call _)) -> ()
  | (_, _, Hole (e, _, _, _)) -> enforce_return_disposable env e
  | (_, p, _) ->
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(primary @@ Primary.Invalid_return_disposable p)

(* Wrappers around the function with the same name in LEnv, which only
 * performs the move/save and merge operation if we are in a try block or in a
 * function with return type 'noreturn'.
 * This enables significant perf improvement, because this is called at every
 * function of method call, when most calls are outside of a try block. *)
let move_and_merge_next_in_catch ~join_pos env =
  if env.in_try || TFTerm.is_noreturn env then
    LEnv.move_and_merge_next_in_cont env ~join_pos C.Catch
  else
    LEnv.drop_cont env C.Next

let save_and_merge_next_in_catch ~join_pos env =
  if env.in_try || TFTerm.is_noreturn env then
    LEnv.save_and_merge_next_in_cont env ~join_pos C.Catch
  else
    env

let might_throw env = save_and_merge_next_in_catch env

let branch :
    type res.
    join_pos:Pos.t ->
    env ->
    (env -> env * res) ->
    (env -> env * res) ->
    env * res * res =
 fun ~join_pos env branch1 branch2 ->
  let parent_lenv = env.lenv in
  let (env, tbr1) = branch1 env in
  let lenv1 = env.lenv in
  let env = { env with lenv = parent_lenv } in
  let (env, tbr2) = branch2 env in
  let lenv2 = env.lenv in
  let env = LEnv.union_lenvs env ~join_pos parent_lenv lenv1 lenv2 in
  (env, tbr1, tbr2)

let as_expr env ty1 pe e =
  let env = Env.open_tyvars env pe in
  let (env, tv) =
    if TUtils.is_tyvar_error env ty1 then
      Env.fresh_type_error env pe
    else
      Env.fresh_type env pe
  in
  let (env, ct) =
    match e with
    | As_v _ ->
      ( env,
        {
          ct_key = None;
          ct_val = tv;
          ct_is_await = false;
          ct_reason = Reason.Rforeach pe;
        } )
    | As_kv _ ->
      let (env, tk) = Env.fresh_type env pe in
      ( env,
        {
          ct_key = Some tk;
          ct_val = tv;
          ct_is_await = false;
          ct_reason = Reason.Rforeach pe;
        } )
    | Await_as_v _ ->
      ( env,
        {
          ct_key = None;
          ct_val = tv;
          ct_is_await = true;
          ct_reason = Reason.Rasyncforeach pe;
        } )
    | Await_as_kv _ ->
      let (env, tk) = Env.fresh_type env pe in
      ( env,
        {
          ct_key = Some tk;
          ct_val = tv;
          ct_is_await = true;
          ct_reason = Reason.Rasyncforeach pe;
        } )
  in
  let expected_ty =
    ConstraintType (mk_constraint_type (Reason.Rforeach pe, Tcan_traverse ct))
  in
  let (ty_actual, is_option) =
    match get_node ty1 with
    | Toption ty_actual -> (ty_actual, true)
    | _ -> (ty1, false)
  in
  let (env, ty_err_opt1) =
    Type.sub_type_i
      pe
      Reason.URforeach
      env
      (LoclType ty_actual)
      expected_ty
      Typing_error.Callback.unify_error
  in
  (* Handle the case where we have a nullable type where the inner type does
     support `Tcan_traverse` *)
  let (ty_mismatch_opt, ty_err_opt2) =
    match ty_err_opt1 with
    | None when is_option ->
      let ty_str = lazy (Typing_print.full_strip_ns env ty1) in
      let ty_ct = SubType.can_traverse_to_iface ct in
      let ct_str = lazy (Typing_print.full_strip_ns env ty_ct) in
      let reasons_opt =
        Some
          Lazy.(
            ty_str >>= fun ty_str ->
            map ct_str ~f:(fun ct_str ->
                let msg = "Expected `" ^ ct_str ^ "` " in
                Reason.to_string msg ct.ct_reason
                @ [
                    ( Pos_or_decl.of_raw_pos pe,
                      Format.sprintf "But got `?%s`" ty_str );
                  ]))
      in
      (* We actually failed so generate the error we should
         have seen *)
      let ty_err =
        Typing_error.(
          primary
          @@ Primary.Unify_error { pos = pe; msg_opt = None; reasons_opt })
      in
      (Some (ty1, ty_actual), Some ty_err)
    | Some _ ->
      let ty_mismatch =
        match mk_ty_mismatch_res ty1 expected_ty ty_err_opt1 with
        | Ok _ -> None
        | Error (act, LoclType exp) -> Some (act, exp)
        | Error (act, ConstraintType _) ->
          Some (act, SubType.can_traverse_to_iface ct)
      in
      (ty_mismatch, None)
    | None -> (None, None)
  in
  let ty_err_opt = Option.merge ty_err_opt1 ty_err_opt2 ~f:Typing_error.both in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  let env = Env.set_tyvar_variance_i env expected_ty in
  let tk =
    match ct.ct_key with
    | None -> MakeType.mixed Reason.Rnone
    | Some tk -> tk
  in
  (Typing_solver.close_tyvars_and_solve env, tk, tv, ty_mismatch_opt)

(* These functions invoke special printing functions for Typing_env. They do not
 * appear in user code, but we still check top level function calls against their
 * names. *)
let typing_env_pseudofunctions =
  SN.PseudoFunctions.(
    String.Hash_set.of_list
      ~growth_allowed:false
      [
        hh_show;
        hh_expect;
        hh_expect_equivalent;
        hh_show_env;
        hh_log_level;
        hh_force_solve;
        hh_loop_forever;
        hh_time;
      ])

let do_hh_expect ~equivalent env use_pos explicit_targs p tys =
  match explicit_targs with
  | [targ] ->
    let ((env, ty_err_opt), (expected_ty, _)) =
      Phase.localize_targ ~check_well_kinded:true env (snd targ)
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    let right_expected_ty =
      if TCO.enable_sound_dynamic (Env.get_tcopt env) then
        MakeType.locl_like
          (Reason.Renforceable (get_pos expected_ty))
          expected_ty
      else
        expected_ty
    in
    (match tys with
    | [expr_ty] ->
      let res =
        SubType.sub_type env expr_ty right_expected_ty
        @@ Some
             Typing_error.(
               Reasons_callback.of_primary_error
               @@ Primary.Hh_expect { pos = p; equivalent })
      in
      let (env, ty_err_opt) =
        match res with
        | (env, None) ->
          if equivalent then
            SubType.sub_type env expected_ty expr_ty
            @@ Some
                 Typing_error.(
                   Reasons_callback.of_primary_error
                   @@ Primary.Hh_expect { pos = p; equivalent })
          else
            (env, None)
        | env_err -> env_err
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      env
    | _ -> env)
  | _ ->
    (Typing_error_utils.add_typing_error ~env
    @@ Typing_error.(
         primary
         @@ Primary.Expected_tparam
              { pos = use_pos; decl_pos = Pos_or_decl.none; n = 1 }));
    env

let loop_forever env =
  (* forever = up to 10 minutes, to avoid accidentally stuck processes *)
  for i = 1 to 600 do
    (* Look up things in shared memory occasionally to have a chance to be
     * interrupted *)
    match Env.get_class env "FOR_TEST_ONLY" with
    | None -> Unix.sleep 1
    | _ -> assert false
  done;
  Utils.assert_false_log_backtrace
    (Some "hh_loop_forever was looping for more than 10 minutes")

let hh_time_start_times = ref SMap.empty

(* Wrap the code you'd like to profile with `hh_time` annotations, e.g.,

     hh_time('start');
     // Expensive to typecheck code
     hh_time('stop');

   `hh_time` admits an optional tag parameter to allow for multiple timings in
   the same file.

     hh_time('start', 'Timing 1');
     // Expensive to typecheck code
     hh_time('stop', 'Timinig 1');

     hh_time('start', 'Timing 2');
     // Some more expensive to typecheck code
     hh_time('stop', 'Timinig 2');

   Limitation: Timings are not scoped, so multiple uses of the same tag
   with `hh_time('start', tag)` will overwrite the beginning time of the
   previous timing.
*)
let do_hh_time el =
  let go command tag =
    match command with
    | Aast.String "start" ->
      let start_time = Unix.gettimeofday () in
      hh_time_start_times := SMap.add tag start_time !hh_time_start_times
    | Aast.String "stop" ->
      let stop_time = Unix.gettimeofday () in
      begin
        match SMap.find_opt tag !hh_time_start_times with
        | Some start_time ->
          let elapsed_time_ms = (stop_time -. start_time) *. 1000. in
          Printf.printf "%s: %0.2fms\n" tag elapsed_time_ms
        | None -> ()
      end
    | _ -> ()
  in
  match el with
  | [(_, (_, _, command))] -> go command "_"
  | [(_, (_, _, command)); (_, (_, _, Aast.String tag))] -> go command tag
  | _ -> ()

let is_parameter env x = Local_id.Map.mem x (Env.get_params env)

let check_escaping_var env (pos, x) =
  let err_opt =
    if Env.is_using_var env x then
      let open Typing_error.Primary in
      Some
        (if Local_id.equal x this then
          Escaping_this pos
        else if is_parameter env x then
          Escaping_disposable_param pos
        else
          Escaping_disposable pos)
    else
      None
  in
  Option.iter err_opt ~f:(fun err ->
      Typing_error_utils.add_typing_error ~env @@ Typing_error.primary err)

let make_result env p te ty =
  (* Set the variance of any type variables that were generated according
   * to how they appear in the expression type *)
  let env = Env.set_tyvar_variance env ty in
  let (env, te) = TUtils.make_simplify_typed_expr env p ty te in
  (env, te, ty)

let localize_targ env ta =
  let pos = fst ta in
  let (env, targ) = Phase.localize_targ ~check_well_kinded:true env ta in
  (env, targ, ExpectedTy.make pos Reason.URhint (fst targ))

let rec set_function_pointer env ty =
  match get_node ty with
  | Tfun ft ->
    let ft = set_ft_is_function_pointer ft true in
    (env, mk (get_reason ty, Tfun ft))
  | Tnewtype (name, _, ty1) when String.equal name SN.Classes.cSupportDyn ->
    let (env, ty1) = set_function_pointer env ty1 in
    TUtils.make_supportdyn (get_reason ty) env ty1
  | _ -> (env, ty)

let rec set_readonly_this pos env ty =
  match get_node ty with
  | Tfun ft ->
    let ft = set_ft_readonly_this ft true in
    (env, mk (get_reason ty, Tfun ft))
  | Tnewtype (name, _, ty1) when String.equal name SN.Classes.cSupportDyn ->
    let (env, ty1) = set_readonly_this pos env ty1 in
    TUtils.make_supportdyn (get_reason ty) env ty1
  | _ -> (env, ty)

let xhp_attribute_decl_ty env sid obj attr =
  let (namepstr, valpty) = attr in
  let (valp, valty) = valpty in
  let ((env, e1), (declty, _tal)) =
    TOG.obj_get
      ~obj_pos:(fst sid)
      ~is_method:false
      ~inst_meth:false
      ~meth_caller:false
      ~nullsafe:None
      ~coerce_from_ty:None
      ~explicit_targs:[]
      ~class_id:(CI sid)
      ~member_id:namepstr
      ~on_error:Typing_error.Callback.unify_error
      env
      obj
  in
  let ureason = Reason.URxhp (snd sid, snd namepstr) in
  let (env, e2) =
    Typing_coercion.coerce_type
      valp
      ureason
      env
      valty
      (MakeType.unenforced declty)
      Typing_error.Callback.xhp_attribute_does_not_match_hint
  in
  let ty_mismatch_opt = mk_ty_mismatch_opt valty declty e2 in
  Option.(
    iter ~f:(Typing_error_utils.add_typing_error ~env)
    @@ merge e1 e2 ~f:Typing_error.both);
  (env, declty, ty_mismatch_opt)

let closure_check_param env param =
  match hint_of_type_hint param.param_type_hint with
  | None -> env
  | Some hty ->
    let hint_pos = fst hty in
    let ((env, ty_err_opt1), hty) = Phase.localize_hint_for_lambda env hty in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
    let paramty = Env.get_local env (Local_id.make_unscoped param.param_name) in
    let (env, ty_err_opt2) =
      Typing_coercion.coerce_type
        hint_pos
        Reason.URhint
        env
        paramty.Typing_local_types.ty
        (MakeType.unenforced hty)
        Typing_error.Callback.unify_error
    in
    Option.iter ty_err_opt2 ~f:(Typing_error_utils.add_typing_error ~env);
    env

let stash_conts_for_closure
    env p is_anon (captured : Typing_local_types.local Aast.capture_lid list) f
    =
  let with_ty_for_lid ((_, name) as lid) = (Env.get_local env name, lid) in

  let captured =
    if is_anon && TCO.any_coeffects (Env.get_tcopt env) then
      Typing_coeffects.(
        with_ty_for_lid (Pos.none, local_capability_id)
        :: with_ty_for_lid (Pos.none, capability_id)
        :: captured)
    else
      captured
  in
  let captured =
    if Env.is_local_present env this && not (Env.is_in_expr_tree env) then
      with_ty_for_lid (Pos.none, this) :: captured
    else
      captured
  in
  let init =
    Option.map (Env.next_cont_opt env) ~f:(fun next_cont ->
        let initial_locals =
          if is_anon then
            Env.get_locals env captured
          else
            next_cont.Typing_per_cont_env.local_types
        in
        let initial_fakes =
          Fake.forget (Env.get_fake_members env) Reason.(Blame (p, BSlambda))
        in
        let tpenv = Env.get_tpenv env in
        (initial_locals, initial_fakes, tpenv))
  in
  LEnv.stash_and_do env (Env.all_continuations env) (fun env ->
      let env =
        match init with
        | None -> env
        | Some (initial_locals, initial_fakes, tpenv) ->
          let env = Env.reinitialize_locals env in
          let env = Env.set_locals env initial_locals in
          let env = Env.set_fake_members env initial_fakes in
          let env = Env.env_with_tpenv env tpenv in
          env
      in
      f env)

let requires_consistent_construct = function
  | CIstatic -> true
  | CIexpr _ -> true
  | CIparent -> false
  | CIself -> false
  | CI _ -> false

(* Caller will be looking for a particular form of expected type
 * e.g. a function type (when checking lambdas) or tuple type (when checking
 * tuples). First expand the expected type and elide single union; also
 * strip nullables, so ?t becomes t, as context will always accept a t if a ?t
 * is expected.
 *
 * If strip_supportdyn is true, then we are expecting a function or shape type for the expected type,
 * and we should decompose supportdyn<t>, and like-push, and return true to
 * indicate that the type supports dynamic.
 *
 * Note: we currently do not generally expand ?t into (null | t), so ~?t is (dynamic | Toption t).
 *)
let expand_expected_and_get_node
    ?(strip_supportdyn = false)
    ~pessimisable_builtin
    env
    (expected : ExpectedTy.t option) =
  let rec unbox ~under_supportdyn env ty =
    let (env, ty) = Env.expand_type env ty in
    match TUtils.try_strip_dynamic env ty with
    | Some stripped_ty ->
      (* We've got a type ty = ~stripped_ty so under Sound Dynamic we need to
       * account for like-pushing: the rule (shown here for vec)
       *     t <:D dynamic
       *     ----------------------
       *     vec<~t> <: ~vec<t>
       * So if expected type is ~vec<t> then we can actually ask for vec<~t>
       * which is more generous.
       *)
      if TCO.enable_sound_dynamic env.genv.tcopt then begin
        let (env, opt_ty) =
          if
            (not strip_supportdyn)
            && TCO.enable_sound_dynamic (Env.get_tcopt env)
            && pessimisable_builtin
          then
            (env, None)
          else
            Typing_dynamic.try_push_like env stripped_ty
        in
        match opt_ty with
        | None -> unbox ~under_supportdyn env stripped_ty
        | Some rty ->
          (* We can only apply like-pushing if the type actually supports dynamic.
           * We know this if we've gone under a supportdyn, *OR* if the type is
           * known to be a dynamic-aware subtype of dynamic. The latter might fail
           * if it's yet to be resolved i.e. a type variable, in which case we just
           * remove the expected type.
           *)
          if under_supportdyn || TUtils.is_supportdyn env rty then
            unbox ~under_supportdyn:true env rty
          else
            (env, None)
      end else
        unbox ~under_supportdyn env stripped_ty
    | None -> begin
      match get_node ty with
      | Tunion [ty] -> unbox ~under_supportdyn env ty
      | Toption ty -> unbox ~under_supportdyn env ty
      | Tnewtype (name, [ty], _) when String.equal name SN.Classes.cSupportDyn
        ->
        let (env, result) = unbox ~under_supportdyn:true env ty in
        begin
          match result with
          | None -> (env, None)
          | Some (ty, _) -> (env, Some (ty, true))
        end
      | _ -> (env, Some (ty, false))
    end
  in
  match expected with
  | None -> (env, None)
  | Some ExpectedTy.{ pos = p; reason = ur; ty = { et_type = ty; _ }; _ } ->
    let (env, res) = unbox ~under_supportdyn:false env ty in
    (match res with
    | None -> (env, None)
    | Some (uty, supportdyn) ->
      if supportdyn && not strip_supportdyn then
        (env, None)
      else
        (env, Some (p, ur, supportdyn, uty, get_node uty)))

let uninstantiable_error env reason_pos cid c_tc_pos c_name c_usage_pos c_ty =
  let reason_ty_opt =
    match cid with
    | CIexpr _ -> Some (reason_pos, lazy (Typing_print.error env c_ty))
    | _ -> None
  in
  let err =
    Typing_error.(
      primary
      @@ Primary.Uninstantiable_class
           {
             pos = c_usage_pos;
             class_name = c_name;
             reason_ty_opt;
             decl_pos = c_tc_pos;
           })
  in
  Typing_error_utils.add_typing_error ~env err

let coerce_to_throwable pos env exn_ty =
  let throwable_ty = MakeType.throwable (Reason.Rthrow pos) in
  Typing_coercion.coerce_type
    ~coerce_for_op:true
    pos
    Reason.URthrow
    env
    exn_ty
    { et_type = throwable_ty; et_enforced = Enforced }
    Typing_error.Callback.unify_error

(* Bind lvar to ty and, if a hint is supplied, ty must be a subtype of it.
   If there is already a binding for lvar, make sure that its explicit
   type bound (if it has one) is respected. *)
let set_valid_rvalue
    ?(is_using_clause = false) ~is_defined p env lvar hint_ty ty =
  let (env, hty, sub_bound_ty) =
    match hint_ty with
    | None -> (env, None, ty)
    | Some hty ->
      let (env, err) =
        Typing_subtype.sub_type
          env
          ty
          hty
          (Some
             Typing_error.(
               Reasons_callback.of_primary_error
               @@ Typing_error.Primary.Unify_error
                    {
                      pos = p;
                      msg_opt =
                        Some
                          "Typed locals can only be assigned values whose type is compatible with the declared type.";
                      reasons_opt = None;
                    }))
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) err;
      (env, Some hty, hty)
  in
  let (env, new_bound_ty, new_ty) =
    if Env.is_local_present env lvar then
      let local = Env.get_local env lvar in
      match local.Typing_local_types.bound_ty with
      | None -> (env, hty, ty)
      | Some old_bound_ty ->
        let (env, err) =
          Typing_subtype.sub_type
            env
            sub_bound_ty
            old_bound_ty
            (Some (Typing_error.Reasons_callback.unify_error_at p))
        in
        (match err with
        | None -> (env, Some (Option.value hty ~default:old_bound_ty), ty)
        | Some err ->
          Typing_error_utils.add_typing_error ~env err;
          (* If the new type or bound violates the old one, then we want to
             check the remainder of the code with the type of the variable
             set to the bound *)
          (env, Some old_bound_ty, old_bound_ty))
    else
      (env, hty, ty)
  in
  let env =
    set_local
      ~is_using_clause
      ~is_defined
      ~bound_ty:new_bound_ty
      env
      (p, lvar)
      new_ty
  in
  (* We are assigning a new value to the local variable, so we need to
   * generate a new expression id
   *)
  match Env.set_local_expr_id env lvar (Env.make_ident env) with
  | Ok env -> env
  | Error (env, err) ->
    Typing_error_utils.add_typing_error ~env err;
    env

let is_hack_collection env ty =
  (* TODO(like types) This fails if a collection is used as a parameter under
   * pessimization, because ~Vector<int> </: ConstCollection<mixed>. This is the
   * test we use to see whether to update the expression id for expressions
   * $vector[0] = $x and $vector[] = $x. If this is false, the receiver is assumed
   * to be a Hack array which are COW. This approximation breaks down in the presence
   * of dynamic. It is unclear whether we should change an expression id if the
   * receiver is dynamic. *)
  match get_node ty with
  | Tvar _ when TUtils.is_tyvar_error env ty -> (true, None)
  | _ ->
    Typing_solver.is_sub_type
      env
      ty
      (MakeType.const_collection Reason.Rnone (MakeType.mixed Reason.Rnone))

let check_class_get
    env p def_pos cid mid ce (_, _cid_pos, e) function_pointer is_method =
  match e with
  | CIself when get_ce_abstract ce -> begin
    match Env.get_self_id env with
    | Some self -> begin
      (* at runtime, self:: in a trait is a call to whatever
       * self:: is in the context of the non-trait "use"-ing
       * the trait's code *)
      match Env.get_class env self with
      | Some cls when Ast_defs.is_c_trait (Cls.kind cls) ->
        (* Ban self::some_abstract_method() in a trait, if the
         * method is also defined in a trait.
         *
         * Abstract methods from interfaces are fine: we'll check
         * in the child class that we actually have an
         * implementation. *)
        (match Decl_provider.get_class (Env.get_ctx env) ce.ce_origin with
        | Some meth_cls when Ast_defs.is_c_trait (Cls.kind meth_cls) ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Self_abstract_call
                   {
                     meth_name = mid;
                     self_pos = _cid_pos;
                     pos = p;
                     decl_pos = def_pos;
                   })
        | _ -> ())
      | _ ->
        (* Ban self::some_abstract_method() in a class. This will
         *  always error. *)
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Self_abstract_call
                 {
                   meth_name = mid;
                   self_pos = _cid_pos;
                   pos = p;
                   decl_pos = def_pos;
                 })
    end
    | None -> ()
  end
  | CIparent when get_ce_abstract ce ->
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Parent_abstract_call
             { meth_name = mid; pos = p; decl_pos = def_pos })
  | CI _ when get_ce_abstract ce && function_pointer ->
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Abstract_function_pointer
             { class_name = cid; meth_name = mid; pos = p; decl_pos = def_pos })
  | CI _ when get_ce_abstract ce ->
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Classname_abstract_call
             { class_name = cid; meth_name = mid; pos = p; decl_pos = def_pos })
  | CI (_, class_name) when get_ce_synthesized ce ->
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Static_synthetic_method
             { class_name; meth_name = mid; pos = p; decl_pos = def_pos })
  | CI (_, class_name) when is_method ->
    (match Env.get_class env class_name with
    | None -> ()
    | Some cd ->
      let req_class = Cls.all_ancestor_req_class_requirements cd in
      if Ast_defs.is_c_trait (Cls.kind cd) && not (List.is_empty req_class) then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Static_call_on_trait_require_class
                 {
                   trait_name = class_name;
                   meth_name = mid;
                   req_class_name =
                     (match
                        TUtils.try_unwrap_class_type
                          (snd (List.hd_exn req_class))
                      with
                     | None -> ""
                     | Some (_r, (_p, req_name), _paraml) -> req_name);
                   pos = p;
                 }))
  | _ -> ()

(** Given an identifier for a function, find its function type in the
 * environment and localise it with the input type parameters.
 *)
let fun_type_of_id env x tal el =
  let (is_echo, lookup_id) =
    (* Strip namespace *)
    if String.equal (snd x) SN.SpecialFunctions.echo then
      (true, SN.PseudoFunctions.echo)
    else
      (false, snd x)
  in
  match Env.get_fun env lookup_id with
  | None ->
    let (env, _, ty) = unbound_name env x ((), Pos.none, Aast.Null) in
    (env, ty, [])
  | Some fd ->
    (match get_node fd.fe_type with
    | Tfun ft ->
      let ft =
        let pessimise = TCO.pessimise_builtins (Env.get_tcopt env) in
        Typing_special_fun.transform_special_fun_ty
          ~pessimise
          ft
          x
          (List.length el)
      in
      let ety_env = empty_expand_env in
      let ((env, ty_err_opt1), tal) =
        Phase.localize_targs
          ~check_well_kinded:true
          ~is_method:true
          ~def_pos:fd.fe_pos
          ~use_pos:(fst x)
          ~use_name:(strip_ns (snd x))
          env
          ft.ft_tparams
          (List.map ~f:snd tal)
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
      let ft =
        Typing_enforceability.compute_enforced_and_pessimize_fun_type
          ~this_class:None
          env
          ft
      in
      let use_pos = fst x in
      let def_pos = fd.fe_pos in
      let ((env, ty_err_opt2), ft) =
        Phase.(
          localize_ft
            ~instantiation:
              { use_name = strip_ns (snd x); use_pos; explicit_targs = tal }
            ~def_pos
            ~ety_env
            env
            ft)
      in
      (* Replace default coeffect by [io] for type of echo
       * (we don't want to make io generally available through hhi)
       * and mark varargs parameter as readonly
       * (this is normally rejected by the parser)
       *)
      let ft =
        if is_echo then
          {
            ft with
            ft_implicit_params =
              {
                capability =
                  CapTy
                    (MakeType.capability
                       (Reason.Rwitness_from_decl fd.fe_pos)
                       SN.Capabilities.io);
              };
            ft_params =
              List.map ft.ft_params ~f:(fun fp ->
                  {
                    fp with
                    fp_flags =
                      Typing_defs_flags.FunParam.set_readonly true fp.fp_flags;
                  });
          }
        else
          ft
      in

      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
      let fty =
        Typing_dynamic.maybe_wrap_with_supportdyn
          ~should_wrap:
            (TCO.enable_sound_dynamic (Env.get_tcopt env)
            && fd.fe_support_dynamic_type)
          (Reason.localize (get_reason fd.fe_type))
          ft
      in
      Option.iter
        ~f:(Typing_error_utils.add_typing_error ~env)
        (TVis.check_deprecated ~use_pos ~def_pos env fd.fe_deprecated);
      Option.iter
        ~f:(Typing_error_utils.add_typing_error ~env)
        (TVis.check_cross_package ~use_pos ~def_pos env ft.ft_cross_package);
      Option.iter
        ~f:(Typing_error_utils.add_typing_error ~env)
        (TVis.check_top_level_access
           ~in_signature:false
           ~use_pos:(fst x)
           ~def_pos:fd.fe_pos
           env
           fd.fe_internal
           (Option.map fd.fe_module ~f:snd));
      (env, fty, tal)
    | _ -> failwith "Expected function type")

(**
 * Checks if a class (given by cty) contains a given static method.
 *
 * We could refactor this + class_get
 *)
let class_contains_smethod env cty (_pos, mid) =
  let lookup_member ty =
    match get_class_type ty with
    | Some ((_, c), _, _) ->
      (match Env.get_class env c with
      | None -> false
      | Some class_ ->
        Option.is_some @@ Env.get_static_member true env class_ mid)
    | None -> false
  in
  let (_env, tyl) =
    TUtils.get_concrete_supertypes ~abstract_enum:true env cty
  in
  List.exists tyl ~f:lookup_member

(* To be a valid trait declaration, all of its 'require extends' must
 * match; since there's no multiple inheritance, it follows that all of
 * the 'require extends' must belong to the same inheritance hierarchy
 * and one of them should be the child of all the others *)
let trait_most_concrete_req_class trait env =
  List.fold
    (Cls.all_ancestor_reqs trait)
    ~f:
      begin
        fun acc (_p, ty) ->
          let (_r, (_p, name), _paraml) = TUtils.unwrap_class_type ty in
          let keep =
            match acc with
            | Some (c, _ty) -> Cls.has_ancestor c name
            | None -> false
          in
          if keep then
            acc
          else
            let class_ = Env.get_class env name in
            match class_ with
            | None -> acc
            | Some c when Ast_defs.is_c_interface (Cls.kind c) -> acc
            | Some c when Ast_defs.is_c_trait (Cls.kind c) ->
              (* this is an error case for which Typing_type_wellformedness spits out
               * an error, but does *not* currently remove the offending
               * 'require extends' or 'require implements' *)
              acc
            | Some c -> Some (c, ty)
      end
    ~init:None

let check_arity ?(did_unpack = false) env pos pos_def ft (arity : int) =
  let exp_min = Typing_defs.arity_min ft in
  if arity < exp_min then
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Typing_too_few_args
             { expected = exp_min; actual = arity; pos; decl_pos = pos_def });
  if get_ft_variadic ft then
    ()
  else
    (* No variadics *)
    let exp_max = List.length ft.ft_params in
    let arity =
      if did_unpack then
        arity + 1
      else
        arity
    in
    if arity > exp_max then
      Typing_error_utils.add_typing_error
        ~env
        Typing_error.(
          primary
          @@ Primary.Typing_too_many_args
               { expected = exp_max; actual = arity; pos; decl_pos = pos_def })

let check_lambda_arity env lambda_pos def_pos lambda_ft expected_ft =
  match (get_ft_variadic lambda_ft, get_ft_variadic expected_ft) with
  | (false, false) ->
    let expected = Typing_defs.arity_min expected_ft in
    let actual = Typing_defs.arity_min lambda_ft in
    let prim_err_opt =
      if actual < expected then
        Some
          (Typing_error.Primary.Typing_too_few_args
             { expected; actual; pos = lambda_pos; decl_pos = def_pos })
      (* Errors.typing_too_few_args expected_min lambda_min lambda_pos def_pos; *)
      else if actual > expected then
        Some
          (Typing_error.Primary.Typing_too_many_args
             { expected; actual; pos = lambda_pos; decl_pos = def_pos })
      (* Errors.typing_too_many_args expected_minlambda_min lambda_pos def_pos *)
      else
        None
    in
    Option.iter prim_err_opt ~f:(fun err ->
        Typing_error_utils.add_typing_error ~env @@ Typing_error.primary err)
  | (_, _) -> ()

(* The variadic capture argument is an array listing the passed
 * variable arguments for the purposes of the function body; callsites
 * should not unify with it *)
let variadic_param env ft =
  if get_ft_variadic ft then
    (env, List.last ft.ft_params)
  else
    (env, None)

let param_modes
    ?(is_variadic = false) ({ fp_pos; _ } as fp) (_, pos, _) param_kind ~env =
  let err_opt =
    let open Typing_error.Primary in
    match (get_fp_mode fp, param_kind) with
    | (FPnormal, Ast_defs.Pnormal) -> None
    | (FPinout, Ast_defs.Pinout _) -> None
    | (FPnormal, Ast_defs.Pinout p) ->
      Some
        (Inout_annotation_unexpected
           {
             pos = Pos.merge p pos;
             decl_pos = fp_pos;
             param_is_variadic = is_variadic;
             qfx_pos = p;
           })
    | (FPinout, Ast_defs.Pnormal) ->
      Some (Inout_annotation_missing { pos; decl_pos = fp_pos })
  in

  Option.iter err_opt ~f:(fun err ->
      Typing_error_utils.add_typing_error ~env @@ Typing_error.primary err)

let split_remaining_params_required_optional ft remaining_params =
  (* Same example as above
   *
   * function f(int $i, string $j, float $k = 3.14, mixed ...$m): void {}
   * function g((string, float, bool) $t): void {
   *   f(3, ...$t);
   * }
   *
   * `remaining_params` will contain [string, float] and there has been 1 parameter consumed. The min_arity
   * of this function is 2, so there is 1 required parameter remaining and 1 optional parameter.
   *)
  let original_params =
    if get_ft_variadic ft then
      List.drop_last_exn ft.ft_params
    else
      ft.ft_params
  in
  let min_arity =
    List.count
      ~f:(fun fp -> not (Typing_defs.get_fp_has_default fp))
      original_params
  in
  let consumed = List.length original_params - List.length remaining_params in
  let required_remaining = Int.max (min_arity - consumed) 0 in
  let (required_params, optional_params) =
    List.split_n remaining_params required_remaining
  in
  (consumed, required_params, optional_params)

let generate_splat_type_vars
    env p required_params optional_params variadic_param =
  let (env, d_required) =
    List.map_env env required_params ~f:(fun env _ -> Env.fresh_type env p)
  in
  let (env, d_optional) =
    List.map_env env optional_params ~f:(fun env _ -> Env.fresh_type env p)
  in
  let (env, d_variadic) =
    match variadic_param with
    | None -> (env, None)
    | Some _ ->
      let (env, ty) = Env.fresh_type env p in
      (env, Some ty)
  in
  (env, (d_required, d_optional, d_variadic))

let check_argument_type_against_parameter_type_helper
    ~dynamic_func env pos param arg_ty =
  Typing_log.(
    log_with_level env "typing" ~level:2 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos pos)
          env
          [
            Log_head
              ( ("Typing.check_argument_type_against_parameter_type_helper "
                ^
                match dynamic_func with
                | None -> "None"
                | Some Supportdyn_function -> "sd"
                | Some Like_function -> "~"),
                [
                  Log_type ("param_ty", param.fp_type.et_type);
                  Log_type ("arg_ty", arg_ty);
                ] );
          ]));
  let param_ty =
    match dynamic_func with
    | Some dyn_func_kind -> begin
      (* It is only sound to add like types to the parameters of supportdyn functions, since
         in this case we are semantically just using the &dynamic part of the type to call them.
         For like functions, they have to check both sides. *)
      match dyn_func_kind with
      | Supportdyn_function ->
        MakeType.locl_like
          (get_reason param.fp_type.et_type)
          param.fp_type.et_type
      | Like_function -> param.fp_type.et_type
    end
    | None -> param.fp_type.et_type
  in
  Typing_coercion.coerce_type
    ~coerce:None
    pos
    Reason.URparam
    env
    arg_ty
    { param.fp_type with et_type = param_ty }
    Typing_error.Callback.unify_error

let check_argument_type_against_parameter_type
    ~dynamic_func
    env
    param
    param_kind
    (((_, pos, expr_) as e : Nast.expr), arg_ty)
    ~is_variadic =
  param_modes ~is_variadic ~env param e param_kind;
  (* When checking params, the type 'x' may be expression dependent. Since
   * we store the expression id in the local env for Lvar, we want to apply
   * it in this case.
   *)
  let (env, dep_ty) =
    match expr_ with
    | Hole ((_, _, Lvar _), _, _, _)
    | Lvar _ ->
      ExprDepTy.make env ~cid:(CIexpr e) arg_ty
    | _ -> (env, arg_ty)
  in
  let pos =
    match param_kind with
    | Ast_defs.Pnormal -> pos
    | Ast_defs.Pinout pk_pos -> Pos.merge pk_pos pos
  in
  let (env, opt_e, used_dynamic) =
    (* First try statically *)
    let (env1, e1opt) =
      check_argument_type_against_parameter_type_helper
        ~dynamic_func:None
        env
        pos
        param
        dep_ty
    in
    if Option.is_some dynamic_func then
      match e1opt with
      | None -> (env1, None, false)
      | Some e1 ->
        let (env2, e2opt) =
          check_argument_type_against_parameter_type_helper
            ~dynamic_func
            env
            pos
            param
            dep_ty
        in
        (match e2opt with
        (* We used dynamic calling to get a successful check *)
        | None -> (env2, None, true)
        (* We failed on both, pick the one with fewest errors! (preferring static on a tie) *)
        | Some e2 ->
          if Typing_error.count e1 <= Typing_error.count e2 then
            (env1, Some e1, false)
          else
            (env2, Some e2, true))
    else
      (env1, e1opt, false)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) opt_e;
  (env, mk_ty_mismatch_opt arg_ty param.fp_type.et_type opt_e, used_dynamic)

let bad_call env p ty =
  if not (TUtils.is_tyvar_error env ty) then
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Bad_call
             { pos = p; ty_name = lazy (Typing_print.error env ty) })

let rec make_a_local_of ~include_this env e =
  match e with
  | (_, p, Class_get ((_, _, cname), CGstring (_, member_name), _)) ->
    let (env, local) = Env.FakeMembers.make_static env cname member_name p in
    (env, Some (p, local))
  | ( _,
      p,
      Obj_get
        ( (((_, _, This) | (_, _, Lvar _)) as obj),
          (_, _, Id (_, member_name)),
          _,
          _ ) ) ->
    let (env, local) = Env.FakeMembers.make env obj member_name p in
    (env, Some (p, local))
  | (_, _, Lvar x)
  | (_, _, Dollardollar x) ->
    (env, Some x)
  | (_, p, This) when include_this -> (env, Some (p, this))
  | (_, _, Hole (e, _, _, _))
  | (_, _, ReadonlyExpr e) ->
    make_a_local_of ~include_this env e
  | _ -> (env, None)

let strip_supportdyn ty =
  (* Currently we don't check for support dynamic at runtime, so we
   * must simply treat supportdyn<t> as t when refining *)
  match get_node ty with
  | Tnewtype (name, [ty], _) when String.equal name SN.Classes.cSupportDyn -> ty
  | _ -> ty

let get_bound_ty_for_lvar env e =
  match e with
  | (_, _, Lvar (_, lid)) ->
    (Typing_env.get_local env lid).Typing_local_types.bound_ty
  | _ -> None

(* This function captures the common bits of logic behind refinement
 * of the type of a local variable or a class member variable as a
 * result of a dynamic check (e.g., nullity check, simple type check
 * using functions like is_int, is_string, is_array etc.).  The
 * argument refine is a function that takes the type of the variable
 * and returns a refined type (making necessary changes to the
 * environment, which is threaded through).
 *
 * All refinement functions return, in addition to the updated
 * environment, a (conservative) set of all the locals that got
 * refined. This set is used to construct AssertEnv statmements in
 * the typed AST.
 *)
let refine_lvalue_type env ((ty, _, _) as te) ~refine =
  let e = Tast.to_nast_expr te in
  let (env, localopt) = make_a_local_of ~include_this:false env e in
  (* TODO TAST: generate an assignment to the fake local in the TAST *)
  match localopt with
  | Some lid ->
    let (env, ty) = refine env ty in
    let bound_ty = get_bound_ty_for_lvar env e in
    (* Refining the type of a typed local shouldn't change it's given bound.
       We assume that the supplied refine function returns a subtype of the
       type it's given, so that the refined type remains a subtype of the
       bound *)
    ( set_local ~is_defined:true ~bound_ty env lid ty,
      Local_id.Set.singleton (snd lid) )
  | None -> (env, Local_id.Set.empty)

let rec condition_nullity ~nonnull (env : env) te =
  match te with
  (* assignment: both the rhs and lhs of the '=' must be made null/non-null *)
  | (_, _, Aast.Binop { bop = Ast_defs.Eq None; lhs = var; rhs = te }) ->
    let (env, lset1) = condition_nullity ~nonnull env te in
    let (env, lset2) = condition_nullity ~nonnull env var in
    (env, Local_id.Set.union lset1 lset2)
  (* case where `Shapes::idx(...)` must be made null/non-null *)
  | ( _,
      _,
      Aast.Call
        {
          func = (_, _, Aast.Class_const ((_, _, Aast.CI (_, shapes)), (_, idx)));
          args = [(Ast_defs.Pnormal, shape); (Ast_defs.Pnormal, field)];
          _;
        } )
    when String.equal shapes SN.Shapes.cShapes && String.equal idx SN.Shapes.idx
    ->
    let field = Tast.to_nast_expr field in
    let refine env shape_ty =
      if nonnull then
        Typing_shapes.shapes_idx_not_null env shape_ty field
      else
        (env, shape_ty)
    in
    refine_lvalue_type env shape ~refine
  | (_, _, Hole (te, _, _, _)) -> condition_nullity ~nonnull env te
  | (_, p, _) ->
    let refine env ty =
      if nonnull then
        Typing_solver.non_null env (Pos_or_decl.of_raw_pos p) ty
      else
        let r = Reason.Rwitness_from_decl (get_pos ty) in
        Inter.intersect env ~r ty (MakeType.null r)
    in
    refine_lvalue_type env te ~refine

(** If we are dealing with a refinement like
      $x is MyClass<A, B>
    then class_info is the class info of MyClass and hint_tyl corresponds
    to A, B. *)
let generate_fresh_tparams env class_info p reason hint_tyl =
  let tparams_len = List.length (Cls.tparams class_info) in
  let hint_tyl = List.take hint_tyl tparams_len in
  let pad_len = tparams_len - List.length hint_tyl in
  let hint_tyl =
    List.map hint_tyl ~f:(fun x -> Some x)
    @ List.init pad_len ~f:(fun _ -> None)
  in
  let replace_wildcard env hint_ty tp =
    let {
      tp_name = (_, tparam_name);
      tp_reified = reified;
      tp_user_attributes;
      _;
    } =
      tp
    in
    let enforceable =
      Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    match hint_ty with
    | Some ty -> begin
      match get_node ty with
      | Tgeneric (name, _targs) when Env.is_fresh_generic_parameter name ->
        (* TODO(T69551141) handle type arguments above and below *)
        (env, (Some (tp, name), MakeType.generic reason name))
      | _ -> (env, (None, ty))
    end
    | None ->
      let (env, new_name) =
        Env.add_fresh_generic_parameter
          env
          (Pos_or_decl.of_raw_pos p)
          tparam_name
          ~reified
          ~enforceable
          ~newable
      in
      (* TODO(T69551141) handle type arguments for Tgeneric *)
      (env, (Some (tp, new_name), MakeType.generic reason new_name))
  in
  let (env, tparams_and_tyl) =
    List.map2_env env hint_tyl (Cls.tparams class_info) ~f:replace_wildcard
  in
  let (tparams_with_new_names, tyl_fresh) = List.unzip tparams_and_tyl in
  (env, tparams_with_new_names, tyl_fresh)

let safely_refine_class_type
    env
    p
    class_name
    class_info
    ivar_ty
    obj_ty
    reason
    (tparams_with_new_names : (decl_tparam * string) option list)
    tyl_fresh =
  (* Type of variable in block will be class name
   * with fresh type parameters *)
  let obj_ty =
    mk (get_reason obj_ty, Tclass (class_name, nonexact, tyl_fresh))
  in
  let tparams = Cls.tparams class_info in
  (* Add in constraints as assumptions on those type parameters *)
  let ety_env =
    {
      empty_expand_env with
      substs = Subst.make_locl tparams tyl_fresh;
      this_ty = obj_ty;
    }
  in
  let add_bounds env (t, ty_fresh) =
    List.fold_left t.tp_constraints ~init:env ~f:(fun env (ck, ty) ->
        (* Substitute fresh type parameters for
         * original formals in constraint *)
        let ((env, ty_err_opt), ty) = Phase.localize ~ety_env env ty in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        SubType.add_constraint env ck ty_fresh ty
        @@ Some (Typing_error.Reasons_callback.unify_error_at p))
  in
  let env =
    List.fold_left (List.zip_exn tparams tyl_fresh) ~f:add_bounds ~init:env
  in
  (* Finally, if we have a class-test on something with static classish type,
   * then we can chase the hierarchy and decompose the types to deduce
   * further assumptions on type parameters. For example, we might have
   *   class B<Tb> { ... }
   *   class C extends B<int>
   * and have obj_ty = C and x_ty = B<T> for a generic parameter Aast.
   * Then SubType.add_constraint will deduce that T=int and add int as
   * both lower and upper bound on T in env.lenv.tpenv
   *
   * We only wish to do this if the types are in a possible subtype relationship.
   *)
  let (env, supertypes) =
    TUtils.get_concrete_supertypes
      ~abstract_enum:true
      ~include_case_types:true
      env
      ivar_ty
  in
  let rec might_be_supertype env ty =
    let (_env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tclass ((_, name), _, _)
      when String.equal name (Cls.name class_info)
           || Cls.has_ancestor class_info name
           || Cls.requires_ancestor class_info name ->
      true
    | Tnewtype (name, tyl, _) ->
      (* For case types we want to open the union, filtering it to only the
       * variant types that share the same data type as [obj_ty] *)
      let (env, variants_opt) =
        Typing_case_types.get_variant_tys env name tyl
      in
      begin
        match variants_opt with
        | Some variants ->
          let (env, ty) =
            Typing_case_types.filter_variants_using_datatype
              env
              (get_reason ty)
              variants
              obj_ty
          in
          might_be_supertype env ty
        | None -> false
      end
    | Toption ty -> might_be_supertype env ty
    | Tunion tyl -> List.for_all tyl ~f:(might_be_supertype env)
    | _ -> false
  in
  let env =
    List.fold_left supertypes ~init:env ~f:(fun env ty ->
        if might_be_supertype env ty then
          SubType.add_constraint env Ast_defs.Constraint_as obj_ty ty
          @@ Some (Typing_error.Reasons_callback.unify_error_at p)
        else
          env)
  in
  (* It's often the case that the fresh name isn't necessary. For
   * example, if C<T> extends B<T>, and we have $x:B<t> for some type t
   * then $x is C should refine to $x:C<t>.
   * We take a simple approach:
   *    For a fresh type parameter T#1, if
   *      - There is an eqality constraint T#1 = t,
   *      - T#1 is covariant, and T#1 has upper bound t (or mixed if absent)
   *      - T#1 is contravariant, and T#1 has lower bound t (or nothing if absent)
   *    then replace T#1 with t.
   * This is done in Type_parameter_env_ops.simplify_tpenv
   *)
  let (env, tparam_substs) =
    Type_parameter_env_ops.simplify_tpenv
      env
      (List.zip_exn tparams_with_new_names tyl_fresh)
      reason
  in
  let tyl_fresh =
    List.map2_exn tyl_fresh tparams_with_new_names ~f:(fun orig_ty tparam_opt ->
        match tparam_opt with
        | None -> orig_ty
        | Some (_tp, name) -> SMap.find name tparam_substs)
  in
  let obj_ty_simplified =
    mk (get_reason obj_ty, Tclass (class_name, nonexact, tyl_fresh))
  in
  (env, obj_ty_simplified)

(** Transform a hint like `A<_>` to a localized type like `A<T#1>` for refinement of
an instance variable. ivar_ty is the previous type of that instance variable. Return
the intersection of the hint and variable. Returns the env, refined type, and a boolean
true if ivar_ty was a class or tuple containing a class and false otherwise. *)
let rec class_for_refinement env p reason ivar_pos ivar_ty hint_ty =
  let (env, hint_ty) = Env.expand_type env hint_ty in
  match (get_node ivar_ty, get_node hint_ty) with
  | (_, Tclass (((_, cid) as _c), _, tyl)) -> begin
    match Env.get_class env cid with
    | Some class_info ->
      let (env, tparams_with_new_names, tyl_fresh) =
        generate_fresh_tparams env class_info p reason tyl
      in
      let (env, ty) =
        safely_refine_class_type
          env
          p
          _c
          class_info
          ivar_ty
          hint_ty
          reason
          tparams_with_new_names
          tyl_fresh
      in
      (env, (ty, true))
    | None -> (env, (MakeType.nothing (Reason.Rmissing_class ivar_pos), true))
  end
  | (Ttuple ivar_tyl, Ttuple hint_tyl)
    when Int.equal (List.length ivar_tyl) (List.length hint_tyl) ->
    let (env, tyl) =
      List.map2_env env ivar_tyl hint_tyl ~f:(fun env ivar_ty hint_ty ->
          class_for_refinement env p reason ivar_pos ivar_ty hint_ty)
    in
    (env, (MakeType.tuple reason (List.map ~f:fst tyl), List.exists ~f:snd tyl))
  | _ -> (env, (hint_ty, false))

let refine_and_simplify_intersection
    ~hint_first env p reason ivar_pos ivar_ty hint_ty =
  let intersect ~hint_first ~is_class env r ty hint_ty =
    let (env, hint_ty) =
      if
        is_class
        && TCO.enable_sound_dynamic (Env.get_tcopt env)
        && TUtils.is_supportdyn env ty
      then
        TUtils.make_supportdyn reason env hint_ty
      else
        (env, hint_ty)
    in
    (* Sometimes the type checker is sensitive to the ordering of intersections *)
    if hint_first then
      Inter.intersect env ~r hint_ty ty
    else
      Inter.intersect env ~r ty hint_ty
  in
  let like_type_simplify ty =
    (* Distribute the intersection over the union *)
    let (env, (hint_ty, is_class)) =
      class_for_refinement env p reason ivar_pos ty hint_ty
    in
    let (env, ty2) =
      intersect ~hint_first:false ~is_class env reason ty hint_ty
    in
    let rec is_enforced hint_ty =
      match get_node hint_ty with
      | Toption ty -> is_enforced ty
      | Tclass (_, _, [])
      | Tprim _ ->
        true
      | _ -> false
    in
    (* If the hint is fully enforced, keep that information around *)
    if is_enforced hint_ty then
      let (env, dyn_ty) =
        Inter.intersect env ~r:reason (MakeType.dynamic reason) hint_ty
      in
      Union.union env dyn_ty ty2
    else
      (env, MakeType.locl_like reason ty2)
  in
  match TUtils.try_strip_dynamic env ivar_ty with
  | Some ty when TCO.enable_sound_dynamic (Env.get_tcopt env) ->
    like_type_simplify ty
  | _ ->
    let (env, (hint_ty, is_class)) =
      class_for_refinement env p reason ivar_pos ivar_ty hint_ty
    in
    intersect ~hint_first ~is_class env reason ivar_ty hint_ty

let ish_weakening env hint hint_ty =
  match hint with
  | (_, Aast.Happly ((_, name), _)) ->
    let enum_opt = Option.(Env.get_enum env name >>= Cls.enum_type) in
    begin
      match enum_opt with
      | Some { te_base; _ } -> begin
        match Typing_defs.get_node te_base with
        | Typing_defs.(Tprim Tarraykey) -> hint_ty
        | _ ->
          MakeType.intersection
            Reason.Rnone
            [
              MakeType.locl_like Reason.Rnone hint_ty;
              MakeType.arraykey Reason.Rnone;
            ]
      end
      | _ -> hint_ty
    end
  | _ -> hint_ty

let refine_for_is ~hint_first env tparamet ivar reason hint =
  let (env, lset) =
    match snd hint with
    | Aast.Hnonnull -> condition_nullity ~nonnull:tparamet env ivar
    | Aast.Hprim Tnull -> condition_nullity ~nonnull:(not tparamet) env ivar
    | _ -> (env, Local_id.Set.empty)
  in
  let (env, locl) =
    make_a_local_of ~include_this:true env (Tast.to_nast_expr ivar)
  in
  match locl with
  | Some locl_ivar ->
    let ((env, ty_err_opt), hint_ty) =
      Phase.localize_hint_for_refinement env hint
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    let hint_ty = strip_supportdyn hint_ty in
    let hint_ty =
      if Env.get_tcopt env |> TCO.pessimise_builtins then
        ish_weakening env hint hint_ty
      else
        hint_ty
    in
    let (env, hint_ty) =
      if not tparamet then
        Inter.negate_type env reason hint_ty ~approx:TUtils.ApproxUp
      else
        (env, hint_ty)
    in
    let (env, refined_ty) =
      refine_and_simplify_intersection
        ~hint_first
        env
        (fst hint)
        reason
        (fst locl_ivar)
        (fst3 ivar)
        hint_ty
    in
    let bound_ty = get_bound_ty_for_lvar env ivar in
    ( set_local ~is_defined:true ~bound_ty env locl_ivar refined_ty,
      Local_id.Set.singleton (snd locl_ivar) )
  | None -> (env, lset)

let refine_for_equality pos env te ty =
  let (env, locl) =
    make_a_local_of ~include_this:true env (Tast.to_nast_expr te)
  in
  match locl with
  | Some locl_ivar ->
    let (env, refined_ty) =
      refine_and_simplify_intersection
        ~hint_first:false
        env
        pos
        (Reason.Requal pos)
        (fst locl_ivar)
        (fst3 te)
        ty
    in
    let bound_ty = get_bound_ty_for_lvar env te in
    ( set_local ~is_defined:true ~bound_ty env locl_ivar refined_ty,
      Local_id.Set.singleton (snd locl_ivar) )
  | None -> (env, Local_id.Set.empty)

type legacy_arrays =
  | HackDictOrDArray
  | HackVecOrVArray

(* Refine type for is_array, is_vec, is_keyset and is_dict tests
 * `pred_name` is the function name itself (e.g. 'is_vec')
 * `p` is position of the function name in the source
 * `arg_expr` is the argument to the function
 *)
let safely_refine_is_array env ty p pred_name arg_expr =
  refine_lvalue_type env arg_expr ~refine:(fun env arg_ty ->
      let r = Reason.Rpredicated (p, pred_name) in
      let (env, tarrkey_name) =
        Env.add_fresh_generic_parameter
          env
          (Pos_or_decl.of_raw_pos p)
          "Tk"
          ~reified:Erased
          ~enforceable:false
          ~newable:false
      in
      (* TODO(T69551141) handle type arguments for Tgeneric *)
      let tarrkey = MakeType.generic r tarrkey_name in
      let env =
        SubType.add_constraint
          env
          Ast_defs.Constraint_as
          tarrkey
          (MakeType.arraykey r)
        @@ Some (Typing_error.Reasons_callback.unify_error_at p)
      in
      let (env, tfresh_name) =
        Env.add_fresh_generic_parameter
          env
          (Pos_or_decl.of_raw_pos p)
          "T"
          ~reified:Erased
          ~enforceable:false
          ~newable:false
      in
      (* TODO(T69551141) handle type arguments for Tgeneric *)
      let tfresh = MakeType.generic r tfresh_name in
      (* This is the refined type of e inside the branch *)
      let hint_ty =
        match ty with
        | HackDictOrDArray -> MakeType.dict r tarrkey tfresh
        | HackVecOrVArray -> MakeType.vec r tfresh
      in
      let (_, arg_pos, _) = arg_expr in
      let (env, (refined_ty, _)) =
        class_for_refinement env p r arg_pos arg_ty hint_ty
      in
      (* Add constraints on generic parameters that must
       * hold for refined_ty <:arg_ty. For example, if arg_ty is Traversable<T>
       * and refined_ty is keyset<T#1> then we know T#1 <: T.
       * See analogous code in safely_refine_class_type.
       *)
      let (env, supertypes) =
        TUtils.get_concrete_supertypes
          ~expand_supportdyn:false
          ~abstract_enum:true
          env
          arg_ty
      in
      let env =
        List.fold_left supertypes ~init:env ~f:(fun env ty ->
            SubType.add_constraint env Ast_defs.Constraint_as hint_ty ty
            @@ Some (Typing_error.Reasons_callback.unify_error_at p))
      in
      Inter.intersect ~r env refined_ty arg_ty)

let key_exists env pos shape field =
  let field = Tast.to_nast_expr field in
  refine_lvalue_type env shape ~refine:(fun env shape_ty ->
      let (fld_opt, ty_err_opt) =
        TUtils.shape_field_name_with_ty_err env field
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      match fld_opt with
      | None -> (env, shape_ty)
      | Some field_name ->
        let field_name = TShapeField.of_ast Pos_or_decl.of_raw_pos field_name in
        Typing_shapes.refine_shape field_name pos env shape_ty)

(** Add a fresh type parameter to [env] with a name starting [prefix]
    and a constraint on [ty]. *)
let synthesize_type_param env p prefix ty =
  let (env, name) =
    match get_node ty with
    (* Don't generate another fresh generic off an existing fresh generic *)
    | Tgeneric (n, _) when Env.is_fresh_generic_parameter n -> (env, n)
    | _ ->
      let (env, name) = Env.fresh_param_name env prefix in
      let env = Env.add_upper_bound_global env name ty in
      let env = Env.add_lower_bound_global env name ty in
      (env, name)
  in
  let hint = (p, Aast.Habstr (name, [])) in
  (hint, env)

(** Transform calls to MyVisitor::makeTree with [f]. *)
let rec rewrite_expr_tree_maketree env expr f =
  let (pos, p, expr_) = expr in
  let (env, expr_) =
    match expr_ with
    | Call
        ({
           func =
             ( fun_pos,
               p,
               (Lfun (fun_, idl) | Efun { ef_fun = fun_; ef_use = idl; _ }) );
           _;
         } as call_expr) ->
      (* Express tree literals containing splices use an anonymous
         function that returns the makeTree call.

          (function() {
             $0splice1 = "whatever";
             return MyVisitor::makeTree(...);
          })()
      *)
      let map_stmt env s =
        match s with
        | (pos, Return (Some expr)) ->
          let (env, expr) = rewrite_expr_tree_maketree env expr f in
          (env, (pos, Return (Some expr)))
        | _ -> (env, s)
      in

      let (env, body_ast) = List.map_env env fun_.f_body.fb_ast ~f:map_stmt in
      let fun_ = { fun_ with f_body = { fb_ast = body_ast } } in

      (env, Call { call_expr with func = (fun_pos, p, Lfun (fun_, idl)) })
    | Call _ ->
      (* The desugarer might give us a simple call to makeTree, so we
         can process it immediately. *)
      f env expr_
    | _ -> (env, expr_)
  in
  (env, (pos, p, expr_))

(** Given [expr], a runtime expression for an expression tree, add a
    type parameter to the makeTree call.

    This enables expression tree visitors to use phantom type
    parameters. The visitor can be defined with __Explicit.

        public static function makeTree<<<__Explicit>> TInfer>(...) { ... }

     Userland calls to this method must provide an explicit type.

        MyVisitor::makeTree<MyVisitorInt>(...);

    For expression tree literals, we run type inference and provide a
    synthesized type parameter to the desugared runtime expression.

        MyVisitor`1`; // we infer MyVisitorInt
        // we add this constrained type parameter:
        MyVisitor::makeTree<TInfer#1>(...) where TInfer#1 = MyVisitorInt

 *)
let maketree_with_type_param env p expr expected_ty =
  let (hint_virtual, env) = synthesize_type_param env p "TInfer" expected_ty in
  let rewrite_expr env expr =
    match expr with
    | Call e -> (env, Call { e with targs = [((), hint_virtual)] })
    | e -> (env, e)
  in

  rewrite_expr_tree_maketree env expr rewrite_expr

module EnumClassLabelOps = struct
  type result =
    | Success of Tast.expr * locl_ty
    | ClassNotFound
    | LabelNotFound of Tast.expr * locl_ty
    | Skip

  (** Given an [enum_id] and a [label_name], tries to see if
      [enum_id] has a constant named [label_name].
      In such case, creates the expected typed expression.

      If [label_name] is not there, it will register an error.

      [ctor] is either `MemberOf` or `Label`
      [full] describes if the original expression was a full
      label, as in E#A, or a short version, as in #A
  *)
  let expand
      pos env ~full ~ctor enum_id label_name (ty_pos : Pos_or_decl.t option) =
    let (_, enum_name) = enum_id in
    let cls = Env.get_class env enum_name in
    match cls with
    | Some cls ->
      (match Env.get_const env cls label_name with
      | Some const_def ->
        let dty = const_def.cc_type in
        (* the enum constant has type MemberOf<X, Y>. If we are
         * processing a Label argument, we just switch MemberOf for
         * Label.
         *)
        let dty =
          match deref dty with
          | (r, Tapply ((p, _), args)) -> mk (r, Tapply ((p, ctor), args))
          | _ -> dty
        in
        let ((env, ty_err_opt), lty) =
          Phase.localize_no_subst env ~ignore_errors:true dty
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        let hi = lty in
        let qualifier =
          if full then
            Some enum_id
          else
            None
        in
        let te = (hi, pos, Aast.EnumClassLabel (qualifier, label_name)) in
        (env, Success (te, lty))
      | None ->
        let consts =
          Cls.consts cls
          |> List.filter ~f:(fun (name, _) ->
                 not (String.equal name SN.Members.mClass))
        in
        let most_similar =
          match Env.most_similar label_name consts fst with
          | Some (name, const) ->
            Some
              ( (if full then
                  Render.strip_ns enum_name ^ "#" ^ name
                else
                  "#" ^ name),
                const.cc_pos )
          | None -> None
        in
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            enum
            @@ Primary.Enum.Enum_class_label_unknown
                 {
                   pos;
                   label_name;
                   enum_name;
                   decl_pos = Cls.pos cls;
                   most_similar;
                   ty_pos;
                 });
        let (env, ty) = Env.fresh_type_error env pos in
        let te = (ty, pos, Aast.EnumClassLabel (None, label_name)) in
        (env, LabelNotFound (te, ty)))
    | None -> (env, ClassNotFound)
end

module Valkind = struct
  type t =
    | Lvalue
    | Lvalue_subexpr
    | Other

  let is_lvalue = function
    | Lvalue -> true
    | Lvalue_subexpr
    | Other ->
      false
end

module rec Expr : sig
  module Config : sig
    type t = {
      accept_using_var: bool;
      check_defined: bool;
    }

    val default : t
  end

  module Context : sig
    type t = {
      is_using_clause: bool;
      valkind: Valkind.t;
      is_attribute_param: bool;
      in_await: locl_phase Reason.t_ option;
      lhs_of_null_coalesce: bool;
    }

    val default : t
  end

  val expr :
    expected:ExpectedTy.t option ->
    config:Config.t ->
    ctxt:Context.t ->
    env ->
    Nast.expr ->
    env * Tast.expr * locl_ty

  val expr_with_pure_coeffects :
    expected:ExpectedTy.t option ->
    env ->
    Nast.expr ->
    env * Tast.expr * locl_ty

  val raw_expr :
    expected:ExpectedTy.t option ->
    config:Config.t ->
    ctxt:Context.t ->
    env ->
    Nast.expr ->
    env * Tast.expr * locl_ty

  val exprs :
    expected:ExpectedTy.t option ->
    config:Config.t ->
    ctxt:Context.t ->
    env ->
    Nast.expr list ->
    env * Tast.expr list * locl_phase Typing_defs_core.ty list

  val lvalue : env -> Nast.expr -> env * Tast.expr * locl_ty

  (** Build an environment for the true or false branch of
  conditional statements. *)
  val condition : env -> bool -> Tast.expr -> env * branch_info

  (** Typechecks a call.
  Returns in this order the typed expressions for the arguments, for the
  variadic arguments, the return type, and a boolean indicating whether fake
  members should be forgotten. If dynamic_func is not None, then we are trying
  to call the function with dynamic arguments using the fact that is is a SDT
  function. That is, we have already ruled out trying to call it with just its
  declared type. *)
  val call :
    expected:ExpectedTy.t option ->
    ?nullsafe:pos option ->
    in_await:locl_phase Reason.t_ option ->
    ?dynamic_func:dyn_func_kind ->
    expr_pos:pos ->
    recv_pos:pos ->
    env ->
    locl_ty ->
    (Ast_defs.param_kind * Nast.expr) list ->
    Nast.expr option ->
    env
    * ((Ast_defs.param_kind * Tast.expr) list
      * Tast.expr option
      * locl_ty
      * bool)

  val new_object :
    expected:ExpectedTy.t option ->
    check_parent:bool ->
    check_not_abstract:bool ->
    is_using_clause:bool ->
    pos ->
    env ->
    unit * pos * (unit, unit) class_id_ ->
    Nast.targ list ->
    (Ast_defs.param_kind * Nast.expr) list ->
    Nast.expr option ->
    env
    * Tast.class_id
    * Tast.targ list
    * (Ast_defs.param_kind * Tast.expr) list
    * Tast.expr option
    * locl_ty
    * locl_ty
    * bool

  val inout_write_back :
    env ->
    locl_ty Typing_defs_core.fun_param ->
    Ast_defs.param_kind * Nast.expr ->
    env

  val update_array_type :
    lhs_of_null_coalesce:bool ->
    pos ->
    env ->
    Nast.expr ->
    Valkind.t ->
    env * Tast.expr * locl_ty
end = struct
  module Config = struct
    type t = {
      accept_using_var: bool;
      check_defined: bool;
    }

    let default = { accept_using_var = false; check_defined = true }
  end

  module Context = struct
    type t = {
      is_using_clause: bool;
      valkind: Valkind.t;
      is_attribute_param: bool;
      in_await: locl_phase Reason.t_ option;
      lhs_of_null_coalesce: bool;
    }

    let default =
      {
        is_using_clause = false;
        valkind = Valkind.Other;
        is_attribute_param = false;
        in_await = None;
        lhs_of_null_coalesce = false;
      }
  end

  let coerce_nonlike_and_like
      ~coerce_for_op ~coerce env pos reason ty ety ety_like =
    let (env1, err1) =
      Typing_coercion.coerce_type
        ~coerce_for_op
        ~coerce
        pos
        reason
        env
        ty
        ety
        Typing_error.Callback.unify_error
    in
    begin
      match err1 with
      | None -> (env1, None, false)
      | Some _ ->
        (* Now check against the pessimised type *)
        let (env2, err2) =
          Typing_coercion.coerce_type
            ~coerce_for_op
            ~coerce
            pos
            reason
            env
            ty
            ety_like
            Typing_error.Callback.unify_error
        in
        (env2, err2, true)
    end

  let rec expr
      ~(expected : ExpectedTy.t option) ~config ~ctxt env ((_, p, _) as e) =
    try
      begin
        match expected with
        | None -> ()
        | Some ExpectedTy.{ reason = r; ty = { et_type = ty; _ }; _ } ->
          Typing_log.(
            log_with_level env "typing" ~level:1 (fun () ->
                log_types
                  (Pos_or_decl.of_raw_pos p)
                  env
                  [
                    Log_head
                      ( "Typing.expr " ^ Reason.string_of_ureason r,
                        [Log_type ("expected_ty", ty)] );
                  ]))
      end;
      raw_expr
        ~expected
        ~config
        ~ctxt:Context.{ ctxt with lhs_of_null_coalesce = false }
        env
        e
    with
    | Inf.InconsistentTypeVarState _ as exn ->
      (* we don't want to catch unwanted exceptions here, eg Timeouts *)
      Errors.exception_occurred p (Exception.wrap exn);
      expr_error env p e
  (*let (env, ty) = Env.fresh_type_error env p in
    make_result env p (invalid_expr_ env p) @@ ty*)

  (* Some (legacy) special functions are allowed in initializers,
     therefore treat them as pure and insert the matching capabilities. *)
  and expr_with_pure_coeffects ~expected env e =
    let (_, p, _) = e in
    let pure = MakeType.mixed (Reason.Rwitness p) in
    let (env, (te, ty)) =
      with_special_coeffects env pure pure @@ fun env ->
      expr env e ~expected ~config:Config.default ~ctxt:Context.default
      |> triple_to_pair
    in
    (env, te, ty)

  and raw_expr ~expected ~config ~ctxt env e =
    let (_, p, _) = e in
    debug_last_pos := p;
    expr_ ~expected ~config ~ctxt env e

  and lvalue env e =
    expr_
      ~expected:None
      ~config:Config.{ check_defined = false; accept_using_var = false }
      ~ctxt:Context.{ default with valkind = Valkind.Lvalue }
      env
      e

  and lvalues env el =
    match el with
    | [] -> (env, [], [])
    | e :: el ->
      let (env, te, ty) = lvalue env e in
      let (env, tel, tyl) = lvalues env el in
      (env, te :: tel, ty :: tyl)

  (* $x ?? 0 is handled similarly to $x ?: 0, except that the latter will also
   * look for sketchy null checks in the condition. *)
  (* TODO TAST: type refinement should be made explicit in the typed AST *)
  and eif env ~(expected : ExpectedTy.t option) ~in_await p c e1 e2 =
    let (env, tc, tyc) =
      raw_expr ~expected:None ~config:Config.default ~ctxt:Context.default env c
    in
    let parent_lenv = env.lenv in
    let (env, _) = condition env true tc in
    let (env, te1, ty1) =
      match e1 with
      | None ->
        let (env, ty) =
          Typing_solver.non_null env (Pos_or_decl.of_raw_pos p) tyc
        in
        (env, None, ty)
      | Some e1 ->
        let (env, te1, ty1) =
          expr
            ~expected
            ~config:Config.default
            ~ctxt:Context.{ default with in_await }
            env
            e1
        in
        (env, Some te1, ty1)
    in
    let lenv1 = env.lenv in
    let env = { env with lenv = parent_lenv } in
    let (env, _) = condition env false tc in
    let (env, te2, ty2) =
      expr
        ~expected
        ~config:Config.default
        ~ctxt:Context.{ default with in_await }
        env
        e2
    in
    let lenv2 = env.lenv in
    let env = LEnv.union_lenvs ~join_pos:p env parent_lenv lenv1 lenv2 in
    let (env, ty) = Union.union ~approx_cancel_neg:true env ty1 ty2 in
    make_result env p (Aast.Eif (tc, te1, te2)) ty

  and exprs ~expected ~config ~ctxt env el =
    match el with
    | [] -> (env, [], [])
    | e :: el ->
      let (env, te, ty) =
        expr
          ~expected
          ~config
          ~ctxt:Context.{ ctxt with is_using_clause = false; in_await = None }
          env
          e
      in
      let (env, tel, tyl) = exprs ~expected ~config ~ctxt env el in
      (env, te :: tel, ty :: tyl)

  and argument_list_exprs expr_cb env el =
    match el with
    | [] -> (env, [], [])
    | (pk, e) :: el ->
      let (env, te, ty) = expr_cb env e in
      let (env, tel, tyl) = argument_list_exprs expr_cb env el in
      (env, (pk, te) :: tel, ty :: tyl)

  and exprs_expected (pos, ur, expected_tyl) env el =
    match (el, expected_tyl) with
    | ([], _) -> (env, [], [])
    | (e :: el, expected_ty :: expected_tyl) ->
      let expected = Some (ExpectedTy.make pos ur expected_ty) in
      let (env, te, ty) =
        expr ~expected ~config:Config.default ~ctxt:Context.default env e
      in
      let (env, tel, tyl) = exprs_expected (pos, ur, expected_tyl) env el in
      (env, te :: tel, ty :: tyl)
    | (el, []) ->
      exprs ~expected:None ~config:Config.default ~ctxt:Context.default env el

  and expr_ ~expected ~config ~ctxt env ((_, p, e) as outer) =
    let env = Env.open_tyvars env p in
    (fun (env, te, ty) ->
      let (env, ty_err_opt) = Typing_solver.close_tyvars_and_solve env in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      (env, te, ty))
    @@
    (*
     * Given an expected type for elements (as extracted from a parameter or return hint,
     * or explicit type argument), a list of expressions, and a function that infers
     * the type of an element, infer an element type for the whole collection.
     *
     * explicit is true if the expected type is just the explicit type argument
     * can_pessimise is true if this is a pessimisable builtin (e.g. Vector)
     * bound is the bound on the type parameter if present (only ever arraykey)
     *)
    let infer_element_type_for_collection
        ~(expected : ExpectedTy.t option)
        ~explicit
        ~reason
        ~can_pessimise
        ~bound
        ~use_pos
        r
        env
        exprs
        expr_infer =
      (* Is this a pessimisable builtin constructor *and* the flag is set? *)
      let do_pessimise_builtin =
        can_pessimise && TCO.pessimise_builtins (Env.get_tcopt env)
      in
      (* Under Sound Dynamic, for pessimisable builtins we add a like to the expected type
       * that is used to check the element expressions.
       *
       * For non-pessimised builtins we also do this if the type is explicit
       * but if an element actually has a like type then this must be reflected
       * in the inferred type argument.
       * e.g. contrast Vector<int>{ $li }  which has type Vector<int>
       *      and      Pair<int,string> { $li, "A" } which has type Pair<~int,string>
       *)
      let non_pessimised_builtin_explicit =
        TCO.pessimise_builtins (Env.get_tcopt env)
        && explicit
        && not do_pessimise_builtin
      in
      (* The supertype is the type argument of the result.
       * If there is an expected type then it's just this, but we might have to
       * add a like to it if it's the explicit type argument and we have a non-pessimised builtin.
       * Otherwise, we generate a fresh type variable and generate constraints against it.
       *)
      let (env, expected_with_implicit_like, supertype) =
        match expected with
        | Some ExpectedTy.{ pos; ty = ety; reason; _ } -> begin
          (* We add an implicit like type to the expected type of elements if
           *   There is an explicit type argument on the collection constructor, or
           *   This is a pessimised builtin (e.g. Vector), or
           *   There is a runtime-enforced bound (because it does no harm to allow dynamic)
           *)
          let (env, like_ty) =
            if
              TCO.enable_sound_dynamic (Env.get_tcopt env)
              && (explicit || do_pessimise_builtin || Option.is_some bound)
            then
              Typing_array_access.pessimise_type env ety.et_type
            else
              (env, ety.et_type)
          in
          let expected_with_implicit_like =
            Some ExpectedTy.(make pos reason like_ty)
          in
          let (env, supertype) =
            if explicit then
              (env, ety.et_type)
            else
              (* Extract the underlying type from the expected type.
               * If it's an intersection, pick up the type under the like
               * e.g. For ~int & arraykey we want to pick up int
               * Otherwise, just strip the like.
               *)
              match get_node ety.et_type with
              | Tintersection tyl ->
                let (had_dynamic, tyl) =
                  List.map_env false tyl ~f:(fun had_dynamic ty ->
                      match TUtils.try_strip_dynamic env ty with
                      | None -> (had_dynamic, ty)
                      | Some ty -> (true, ty))
                in
                if had_dynamic then
                  (* Put the intersection back together without the dynamic *)
                  let (env, ty) =
                    Inter.simplify_intersections
                      env
                      (mk (get_reason ety.et_type, Tintersection tyl))
                  in
                  (env, ty)
                else
                  (env, ety.et_type)
              | _ -> (env, TUtils.strip_dynamic env ety.et_type)
          in
          (env, expected_with_implicit_like, supertype)
        end
        | None ->
          let (env, ty) = Env.fresh_type_reason env use_pos r in
          (env, None, ty)
      in
      (* Actually type the elements, given the expected element type *)
      let (env, exprs_and_tys) =
        List.map_env
          env
          exprs
          ~f:(expr_infer ~expected:expected_with_implicit_like)
      in
      let (exprs, tys) = List.unzip exprs_and_tys in
      let coerce_for_op =
        Option.is_some bound && not (TCO.pessimise_builtins (Env.get_tcopt env))
      in
      (* Take the inferred type ty of each value and assert that it's a subtype
       * of the (optionally with like type added) result type argument.
       *)
      let subtype_value env ty =
        let (env, ty_err_opt, used_dynamic) =
          if non_pessimised_builtin_explicit then
            (* We have an explicit type argument for a non-pessimised builtin.
             * We try first to check the value type against the non-pessimised type argument.
             * If that fails, we try it against a pessimised type argument.
             *)
            match expected_with_implicit_like with
            | Some ExpectedTy.{ pos; reason; ty = ety_like; coerce; _ } ->
              coerce_nonlike_and_like
                ~coerce_for_op
                ~coerce
                env
                pos
                reason
                ty
                (MakeType.unenforced supertype)
                ety_like
              (* Shouldn't happen *)
            | _ -> (env, None, false)
          else
            let (env, ety) =
              if coerce_for_op then
                (env, MakeType.enforced supertype)
              else
                match expected_with_implicit_like with
                | None ->
                  let (env, supertype) =
                    if can_pessimise then
                      Typing_array_access.maybe_pessimise_type env supertype
                    else
                      (env, supertype)
                  in
                  (env, MakeType.unenforced supertype)
                | Some e -> (env, e.ExpectedTy.ty)
            in
            let (env, ty_err_opt) =
              Typing_coercion.coerce_type
                ~coerce_for_op
                ~coerce:None
                use_pos
                reason
                env
                ty
                ety
                Typing_error.Callback.unify_error
            in
            (env, ty_err_opt, false)
        in
        Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
        (env, ty_err_opt, used_dynamic)
      in
      let (env, rev_ty_err_opts, used_dynamic) =
        List.fold_left
          tys
          ~init:(env, [], false)
          ~f:(fun (env, errs, used_dynamic) ty ->
            let (env, ty_err_opt, u) = subtype_value env ty in
            match ty_err_opt with
            | None -> (env, None :: errs, u || used_dynamic)
            | Some _ -> (env, Some (ty, supertype) :: errs, u || used_dynamic))
      in
      (* used_dynamic is set if we had an explicit type argument t that is not
       * a like type, but one of the inferred element types is of the form ~u.
       * In this case we infer ~t as the element type for the whole collection. *)
      let (env, result_type) =
        if used_dynamic then
          Typing_array_access.pessimise_type env supertype
        else
          (env, supertype)
      in
      let (env, result_type) =
        match bound with
        | Some ty -> Inter.intersect env ~r:(get_reason ty) ty result_type
        | None -> (env, result_type)
      in
      let exprs =
        List.map2_exn
          ~f:(fun te ty_mismatch_opt ->
            match te with
            | (_, _, Hole _) -> te
            | _ -> hole_on_ty_mismatch te ~ty_mismatch_opt)
          exprs
          (List.rev rev_ty_err_opts)
      in
      (env, exprs, result_type)
    in
    let check_collection_tparams env name tys =
      (* varrays and darrays are not classes but they share the same
         constraints with vec and dict respectively *)
      let name =
        if String.equal name SN.Typehints.varray then
          SN.Collections.cVec
        else if String.equal name SN.Typehints.darray then
          SN.Collections.cDict
        else
          name
      in
      (* Class retrieval always succeeds because we're fetching a
         collection decl from an HHI file. *)
      match Env.get_class env name with
      | Some class_ ->
        let ety_env =
          {
            (empty_expand_env_with_on_error
               (Env.invalid_type_hint_assert_primary_pos_in_current_decl env))
            with
            substs = TUtils.make_locl_subst_for_class_tparams class_ tys;
          }
        in
        let (env, ty_err_opt) =
          Phase.check_tparams_constraints
            ~use_pos:p
            ~ety_env
            env
            (Cls.tparams class_)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        env
      | None ->
        let desc = "Missing collection decl during type parameter check"
        and telemetry =
          Telemetry.(create () |> string_ ~key:"class name" ~value:name)
        in
        Errors.invariant_violation p telemetry desc ~report_to_user:false;
        (* Continue typechecking without performing the check on a best effort
           basis. *)
        env
    in
    List.iter ~f:(Typing_error_utils.add_typing_error ~env)
    @@ Typing_type_wellformedness.expr env outer;
    match e with
    | Import _
    | Collection _ ->
      failwith "AST should not contain these nodes"
    | Hole (e, _, _, _) ->
      expr_
        ~expected
        ~config
        ~ctxt:Context.{ ctxt with is_attribute_param = false }
        env
        e
    | Invalid expr_opt ->
      let ty = MakeType.nothing @@ Reason.Rinvalid in
      let expr_opt =
        Option.map
          ~f:(Aast.map_expr (fun _ -> ty) (fun _ -> Tast.dummy_saved_env))
          expr_opt
      in
      let expr_ = Aast.Invalid expr_opt in
      make_result env p expr_ ty
    | Omitted ->
      (* If an expression is omitted in an lvalue position, currently only
       * possible in a list destructuring construct such as `list(,$x) = $y;`,
       * we want to assign anything *to* the index of the omitted expression,
       * so give it type `mixed`. If an expression is omitted in an rvalue position,
       * currently only constant declarations (e.g. in hhi files), then we want
       * to be to assign *from* the expression, so give it type `nothing`.
       *)
      let ty =
        match ctxt.Context.valkind with
        | Valkind.Lvalue
        | Valkind.Lvalue_subexpr ->
          MakeType.mixed (Reason.Rwitness p)
        | Valkind.Other -> MakeType.nothing (Reason.Rwitness p)
      in
      make_result env p Aast.Omitted ty
    | Varray (th, el)
    | ValCollection (_, th, el) ->
      let ( get_expected_kind,
            name,
            subtype_val,
            make_expr,
            make_ty,
            key_bound,
            pessimisable_builtin ) =
        match e with
        | ValCollection ((kind_pos, kind), _, _) ->
          let class_name = Nast.vc_kind_to_name kind in
          let (subtype_val, key_bound, pessimisable_builtin) =
            match kind with
            | Set
            | ImmSet ->
              ( arraykey_value p class_name true,
                Some
                  (MakeType.arraykey
                     (Reason.Rtype_variable_generics
                        (p, "Tk", strip_ns class_name))),
                true )
            | Keyset ->
              ( arraykey_value p class_name true,
                Some
                  (MakeType.arraykey
                     (Reason.Rtype_variable_generics
                        (p, "Tk", strip_ns class_name))),
                false )
            | Vector
            | ImmVector ->
              (array_value, None, true)
            | Vec -> (array_value, None, false)
          in
          ( get_vc_inst env p kind,
            class_name,
            subtype_val,
            (fun th elements ->
              Aast.ValCollection ((kind_pos, kind), th, elements)),
            (fun value_ty ->
              MakeType.class_type (Reason.Rwitness p) class_name [value_ty]),
            key_bound,
            pessimisable_builtin )
        | Varray _ ->
          ( get_vc_inst env p Vec,
            "varray",
            array_value,
            (fun th elements -> Aast.ValCollection ((p, Vec), th, elements)),
            (fun value_ty -> MakeType.vec (Reason.Rwitness p) value_ty),
            None,
            false )
        | _ ->
          (* The parent match makes this case impossible *)
          failwith "impossible match case"
      in
      (* Use expected type to determine expected element type *)
      let (env, elem_expected, th) =
        match th with
        | Some (_, tv) ->
          let ((env, ty_err_opt), tv, tv_expected) = localize_targ env tv in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          let env = check_collection_tparams env name [fst tv] in
          (env, Some tv_expected, Some tv)
        | None -> begin
          match
            expand_expected_and_get_node ~pessimisable_builtin env expected
          with
          | (env, Some (pos, ur, _, ety, _)) -> begin
            match get_expected_kind ety with
            | Some (env, vty) -> (env, Some (ExpectedTy.make pos ur vty), None)
            | None -> (env, None, None)
          end
          | _ -> (env, None, None)
        end
      in
      let (env, tel, elem_ty) =
        infer_element_type_for_collection
          ~expected:elem_expected
          ~explicit:(Option.is_some th)
          ~use_pos:p
          ~reason:Reason.URvector
          ~can_pessimise:pessimisable_builtin
          ~bound:key_bound
          (Reason.Rtype_variable_generics (p, "T", strip_ns name))
          env
          el
          subtype_val
      in
      make_result env p (make_expr th tel) (make_ty elem_ty)
    | Darray (th, l)
    | KeyValCollection (_, th, l) ->
      let (get_expected_kind, name, make_expr, make_ty, pessimisable_builtin) =
        match e with
        | KeyValCollection ((kind_pos, kind), _, _) ->
          let class_name = Nast.kvc_kind_to_name kind in
          ( get_kvc_inst env p kind,
            class_name,
            (fun th pairs ->
              Aast.KeyValCollection ((kind_pos, kind), th, pairs)),
            (fun k v ->
              MakeType.class_type (Reason.Rwitness p) class_name [k; v]),
            (match kind with
            | Dict -> false
            | _ -> true) )
        | Darray _ ->
          let name = "darray" in
          ( get_kvc_inst env p Dict,
            name,
            (fun th pairs -> Aast.KeyValCollection ((p, Dict), th, pairs)),
            (fun k v -> MakeType.dict (Reason.Rwitness p) k v),
            false )
        | _ ->
          (* The parent match makes this case impossible *)
          failwith "impossible match case"
      in
      (* Use expected type to determine expected key and value types *)
      let (env, kexpected, vexpected, th) =
        match th with
        | Some ((_, tk), (_, tv)) ->
          let ((env, ty_err_opt1), tk, tk_expected) = localize_targ env tk in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
          let ((env, ty_err_opt2), tv, tv_expected) = localize_targ env tv in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
          let env = check_collection_tparams env name [fst tk; fst tv] in
          (env, Some tk_expected, Some tv_expected, Some (tk, tv))
        | _ -> begin
          (* no explicit typehint, fallback to supplied expect *)
          match
            expand_expected_and_get_node ~pessimisable_builtin env expected
          with
          | (env, Some (pos, reason, _, ety, _)) -> begin
            match get_expected_kind ety with
            | Some (env, kty, vty) ->
              let k_expected = ExpectedTy.make pos reason kty in
              let v_expected = ExpectedTy.make pos reason vty in
              (env, Some k_expected, Some v_expected, None)
            | None -> (env, None, None, None)
          end
          | _ -> (env, None, None, None)
        end
      in
      let (kl, vl) = List.unzip l in
      let r = Reason.Rtype_variable_generics (p, "Tk", strip_ns name) in
      let (env, tkl, k) =
        infer_element_type_for_collection
          ~expected:kexpected
          ~explicit:(Option.is_some th)
          ~use_pos:p
          ~reason:(Reason.URkey name)
          ~bound:(Some (MakeType.arraykey r))
          ~can_pessimise:pessimisable_builtin
          r
          env
          kl
          (arraykey_value p name false)
      in
      let (env, tvl, v) =
        infer_element_type_for_collection
          ~expected:vexpected
          ~explicit:(Option.is_some th)
          ~use_pos:p
          ~reason:(Reason.URvalue name)
          ~can_pessimise:pessimisable_builtin
          ~bound:None
          (Reason.Rtype_variable_generics (p, "Tv", strip_ns name))
          env
          vl
          array_value
      in
      let pairs = List.zip_exn tkl tvl in
      make_result env p (make_expr th pairs) (make_ty k v)
    | Clone e ->
      let (env, te, ty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e
      in
      (* Clone only works on objects; anything else fatals at runtime.
       * Constructing a call `e`->__clone() checks that `e` is an object and
       * checks coeffects on __clone *)
      let (_, pe, _) = e in
      let ((env, ty_err_opt), (tfty, _tal)) =
        TOG.obj_get
          ~obj_pos:pe
          ~is_method:true
          ~inst_meth:false
          ~meth_caller:false
          ~nullsafe:None
          ~coerce_from_ty:None
          ~explicit_targs:[]
          ~class_id:(CIexpr e)
          ~member_id:(p, SN.Members.__clone)
          ~on_error:Typing_error.Callback.unify_error
          env
          ty
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      let (env, (_tel, _typed_unpack_element, _ty, _should_forget_fakes)) =
        call
          ~expected:None
          ~expr_pos:p
          ~recv_pos:p
          ~in_await:None
          env
          tfty
          []
          None
      in
      make_result env p (Aast.Clone te) ty
    | This ->
      if Option.is_none (Env.get_self_ty env) then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.This_var_outside_class p);
      if Env.is_in_expr_tree env then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(expr_tree @@ Primary.Expr_tree.This_var_in_expr_tree p);
      if not config.Config.accept_using_var then check_escaping_var env (p, this);
      let ty =
        if config.Config.check_defined then
          Env.get_local_check_defined env (p, this)
        else
          Env.get_local env this
      in
      let r = Reason.Rwitness p in
      (* $this isn't a typed local and so has no bound *)
      let ty = mk (r, get_node ty.Typing_local_types.ty) in
      make_result env p Aast.This ty
    | True -> make_result env p Aast.True (MakeType.bool (Reason.Rwitness p))
    | False -> make_result env p Aast.False (MakeType.bool (Reason.Rwitness p))
    (* TODO TAST: consider checking that the integer is in range. Right now
     * it's possible for HHVM to fail on well-typed Hack code
     *)
    | Int s -> make_result env p (Aast.Int s) (MakeType.int (Reason.Rwitness p))
    | Float s ->
      make_result env p (Aast.Float s) (MakeType.float (Reason.Rwitness p))
    (* TODO TAST: consider introducing a "null" type, and defining ?t to
     * be null | t
     *)
    | Null -> make_result env p Aast.Null (MakeType.null (Reason.Rwitness p))
    | String s ->
      make_result env p (Aast.String s) (MakeType.string (Reason.Rwitness p))
    | String2 idl ->
      let (env, tel) = string2 env idl in
      make_result env p (Aast.String2 tel) (MakeType.string (Reason.Rwitness p))
    | PrefixedString (n, e) ->
      if String.( <> ) n "re" then (
        Errors.experimental_feature
          p
          "String prefixes other than `re` are not yet supported.";
        expr_error env p outer
      ) else
        let (env, te, ty) =
          expr
            ~expected:None
            ~config:Config.{ config with accept_using_var = false }
            ~ctxt:
              Context.
                { default with is_attribute_param = ctxt.is_attribute_param }
            env
            e
        in
        let (_, pe, expr_) = e in
        let env = Typing_substring.sub_string pe env ty in
        (match expr_ with
        | String _ -> begin
          try
            make_result
              env
              p
              (Aast.PrefixedString (n, te))
              (Typing_regex.type_pattern e)
          with
          | Pcre.Error (Pcre.BadPattern (s, i)) ->
            let reason = `bad_patt (s ^ " [" ^ string_of_int i ^ "]") in
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary @@ Primary.Bad_regex_pattern { pos = pe; reason });
            expr_error env pe e
          | Typing_regex.Empty_regex_pattern ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Bad_regex_pattern { pos = pe; reason = `empty_patt });
            expr_error env pe e
          | Typing_regex.Missing_delimiter ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Bad_regex_pattern
                     { pos = pe; reason = `missing_delim });
            expr_error env pe e
          | Typing_regex.Invalid_global_option ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Bad_regex_pattern
                     { pos = pe; reason = `invalid_option });
            expr_error env pe e
        end
        | String2 _ ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Re_prefixed_non_string
                   { pos = pe; reason = `embedded_expr });
          expr_error env pe e
        | _ ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Re_prefixed_non_string
                   { pos = pe; reason = `non_string });
          expr_error env pe e)
    | Id ((cst_pos, cst_name) as id) ->
      (match Env.get_gconst env cst_name with
      | None ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.Unbound_global cst_pos);
        let (env, ty) = Env.fresh_type_error env cst_pos in
        make_result env cst_pos (Aast.Id id) ty
      | Some const ->
        let ((env, ty_err_opt), ty) =
          Phase.localize_no_subst env ~ignore_errors:true const.cd_type
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        make_result env p (Aast.Id id) ty)
    | Method_caller (((pos, class_name) as pos_cname), meth_name) ->
      (* meth_caller('X', 'foo') desugars to:
       * $x ==> $x->foo()
       *)
      let class_ = Env.get_class env class_name in
      (match class_ with
      | None -> unbound_name env pos_cname outer
      | Some class_ ->
        (* Create a class type for the given object instantiated with unresolved
         * types for its type parameters.
         *)
        let () =
          if Ast_defs.is_c_trait (Cls.kind class_) then
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Meth_caller_trait { pos; trait_name = class_name })
        in
        let (env, tvarl) =
          List.map_env env (Cls.tparams class_) ~f:(fun env _ ->
              Env.fresh_type env p)
        in
        let params =
          List.map (Cls.tparams class_) ~f:(fun { tp_name = (p, n); _ } ->
              (* TODO(T69551141) handle type arguments for Tgeneric *)
              MakeType.generic (Reason.Rwitness_from_decl p) n)
        in
        let obj_type =
          MakeType.apply
            (Reason.Rwitness_from_decl (Pos_or_decl.of_raw_pos p))
            (Positioned.of_raw_positioned pos_cname)
            params
        in
        let ety_env =
          {
            (empty_expand_env_with_on_error
               (Typing_error.Reasons_callback.invalid_type_hint pos))
            with
            substs = TUtils.make_locl_subst_for_class_tparams class_ tvarl;
          }
        in
        let ((env, ty_err_opt1), local_obj_ty) =
          Phase.localize ~ety_env env obj_type
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
        let ((env, ty_err_opt2), (fty, _tal)) =
          TOG.obj_get
            ~obj_pos:pos
            ~is_method:true
            ~nullsafe:None
            ~inst_meth:false
            ~meth_caller:true
            ~coerce_from_ty:None
            ~explicit_targs:[]
            ~class_id:(CI (pos, class_name))
            ~member_id:meth_name
            ~on_error:Typing_error.Callback.unify_error
            env
            local_obj_ty
        in
        Option.iter ty_err_opt2 ~f:(Typing_error_utils.add_typing_error ~env);
        let (env, fty) = Env.expand_type env fty in
        (match deref (strip_supportdyn fty) with
        | (reason, Tfun ftype) ->
          (* We are creating a fake closure:
           * function(Class $x, arg_types_of(Class::meth_name))
                 : return_type_of(Class::meth_name)
           *)
          let ety_env =
            {
              ety_env with
              on_error =
                Some (Env.unify_error_assert_primary_pos_in_current_decl env);
            }
          in
          let (env, ty_err_opt3) =
            Phase.check_tparams_constraints
              ~use_pos:p
              ~ety_env
              env
              (Cls.tparams class_)
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt3;
          let local_obj_fp = TUtils.default_fun_param local_obj_ty in
          let fty =
            { ftype with ft_params = local_obj_fp :: ftype.ft_params }
          in
          let caller =
            {
              ft_tparams = fty.ft_tparams;
              ft_where_constraints = fty.ft_where_constraints;
              ft_params = fty.ft_params;
              ft_implicit_params = fty.ft_implicit_params;
              ft_ret = fty.ft_ret;
              ft_flags = fty.ft_flags;
              ft_cross_package = fty.ft_cross_package;
            }
          in
          let ty =
            Typing_dynamic.maybe_wrap_with_supportdyn
              ~should_wrap:(TCO.enable_sound_dynamic (Env.get_tcopt env))
              reason
              caller
          in
          make_result env p (Aast.Method_caller (pos_cname, meth_name)) ty
        | _ ->
          (* Shouldn't happen *)
          let (env, ty) = Env.fresh_type_error env pos in
          make_result env p (Aast.Method_caller (pos_cname, meth_name)) ty))
    | FunctionPointer (FP_class_const (cid, meth), targs) ->
      let (env, _, ce, cty) = Class_id.class_expr env [] cid in
      let (env, (fpty, tal)) =
        Class_get_expr.class_get
          ~is_method:true
          ~is_const:false
          ~transform_fty:None
          ~incl_tc:false (* What is this? *)
          ~coerce_from_ty:None (* What is this? *)
          ~explicit_targs:targs
          ~is_function_pointer:true
          env
          cty
          meth
          cid
      in
      let env = Env.set_tyvar_variance env fpty in
      let (env, fpty) = set_function_pointer env fpty in
      (* All function pointers are readonly_this since they are either toplevel or static *)
      let (env, fpty) = set_readonly_this p env fpty in
      make_result
        env
        p
        (Aast.FunctionPointer (FP_class_const (ce, meth), tal))
        fpty
    | Lplaceholder p ->
      let r = Reason.Rplaceholder p in
      let ty = MakeType.void r in
      make_result env p (Aast.Lplaceholder p) ty
    | Dollardollar id ->
      let ty = Env.get_local_check_defined env id in
      let env = might_throw ~join_pos:p env in
      make_result env p (Aast.Dollardollar id) ty.Typing_local_types.ty
    | Lvar ((_, x) as id) ->
      if not config.Config.accept_using_var then check_escaping_var env id;
      let ty =
        if config.Config.check_defined then
          Env.get_local_check_defined env id
        else
          Env.get_local env x
      in
      make_result env p (Aast.Lvar id) ty.Typing_local_types.ty
    | Tuple el ->
      let (env, expected) =
        expand_expected_and_get_node ~pessimisable_builtin:false env expected
      in
      let (env, tel, tyl) =
        match expected with
        | Some (pos, ur, _, _, Ttuple expected_tyl) ->
          exprs_expected (pos, ur, expected_tyl) env el
        | _ ->
          exprs
            ~expected:None
            ~config:Config.{ config with accept_using_var = false }
            ~ctxt:
              Context.
                { default with is_attribute_param = ctxt.is_attribute_param }
            env
            el
      in
      let ty = MakeType.tuple (Reason.Rwitness p) tyl in
      make_result env p (Aast.Tuple tel) ty
    | List el ->
      let (env, tel, tyl) =
        match ctxt.Context.valkind with
        | Valkind.Lvalue
        | Valkind.Lvalue_subexpr ->
          lvalues env el
        | Valkind.Other ->
          let (env, expected) =
            expand_expected_and_get_node ~pessimisable_builtin:true env expected
          in
          (match expected with
          | Some (pos, ur, _, _, Ttuple expected_tyl) ->
            exprs_expected (pos, ur, expected_tyl) env el
          | _ ->
            exprs
              ~expected:None
              ~config:Config.{ config with accept_using_var = false }
              ~ctxt:
                Context.
                  { default with is_attribute_param = ctxt.is_attribute_param }
              env
              el)
      in
      let ty = MakeType.tuple (Reason.Rwitness p) tyl in
      make_result env p (Aast.List tel) ty
    | Pair (th, e1, e2) ->
      let (env, expected1, expected2, th) =
        match th with
        | Some ((_, t1), (_, t2)) ->
          let ((env, ty_err_opt1), t1, t1_expected) = localize_targ env t1 in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
          let ((env, ty_err_opt2), t2, t2_expected) = localize_targ env t2 in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
          (env, Some t1_expected, Some t2_expected, Some (t1, t2))
        | None ->
          (* Use expected type to determine expected element types *)
          (match
             expand_expected_and_get_node
               ~pessimisable_builtin:false
               env
               expected
           with
          | (env, Some (pos, reason, _, _ty, Tclass ((_, k), _, [ty1; ty2])))
            when String.equal k SN.Collections.cPair ->
            let ty1_expected = ExpectedTy.make pos reason ty1 in
            let ty2_expected = ExpectedTy.make pos reason ty2 in
            (env, Some ty1_expected, Some ty2_expected, None)
          | _ -> (env, None, None, None))
      in
      let (_, p1, _) = e1 in
      let (_, p2, _) = e2 in
      let (env, tel1, ty1) =
        infer_element_type_for_collection
          ~expected:expected1
          ~explicit:(Option.is_some th)
          ~can_pessimise:false
          ~use_pos:p1
          ~reason:Reason.URpair_value
          ~bound:None
          (Reason.Rtype_variable_generics (p1, "T1", "Pair"))
          env
          [e1]
          array_value
      in
      let (env, tel2, ty2) =
        infer_element_type_for_collection
          ~expected:expected2
          ~explicit:(Option.is_some th)
          ~can_pessimise:false
          ~use_pos:p2
          ~reason:Reason.URpair_value
          ~bound:None
          (Reason.Rtype_variable_generics (p2, "T2", "Pair"))
          env
          [e2]
          array_value
      in
      let ty = MakeType.pair (Reason.Rwitness p) ty1 ty2 in
      let te1 = List.hd_exn tel1 in
      let te2 = List.hd_exn tel2 in
      make_result env p (Aast.Pair (th, te1, te2)) ty
    | Array_get (e, None) ->
      let (env, te, _) =
        update_array_type
          p
          env
          e
          ctxt.Context.valkind
          ~lhs_of_null_coalesce:false
      in
      let env = might_throw ~join_pos:p env in
      (* NAST check reports an error if [] is used for reading in an
           lvalue context. *)
      let (env, ty) = Env.fresh_type_error env p in
      make_result env p (Aast.Array_get (te, None)) ty
    | Array_get (e1, Some e2) ->
      let Context.{ is_attribute_param; lhs_of_null_coalesce; valkind; _ } =
        ctxt
      in
      let (env, te1, ty1) =
        update_array_type ~lhs_of_null_coalesce p env e1 valkind
      in
      let (env, te2, ty2) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:Context.{ default with is_attribute_param }
          env
          e2
      in
      let env = might_throw ~join_pos:p env in
      let is_lvalue = Valkind.is_lvalue valkind in
      let (_, p1, _) = e1 in
      let (env, (ty, arr_ty_mismatch_opt, key_ty_mismatch_opt)) =
        Typing_array_access.array_get
          ~expr_ty:ty1
          ~array_pos:p1
          ~expr_pos:p
          ~lhs_of_null_coalesce
          is_lvalue
          env
          ty1
          e2
          ty2
      in
      make_result
        env
        p
        (Aast.Array_get
           ( hole_on_ty_mismatch ~ty_mismatch_opt:arr_ty_mismatch_opt te1,
             Some (hole_on_ty_mismatch ~ty_mismatch_opt:key_ty_mismatch_opt te2)
           ))
        ty
    | Call
        { func = (_, pos_id, Id (_, s)) as e; targs; args; unpacked_arg = None }
      when Hash_set.mem typing_env_pseudofunctions s ->
      let (env, _tel, tys) =
        argument_list_exprs
          (expr
             ~expected:None
             ~config:Config.{ config with accept_using_var = true }
             ~ctxt:
               Context.
                 { default with is_attribute_param = ctxt.is_attribute_param })
          env
          args
      in
      let env =
        if String.equal s SN.PseudoFunctions.hh_expect then
          do_hh_expect ~equivalent:false env pos_id targs p tys
        else if String.equal s SN.PseudoFunctions.hh_expect_equivalent then
          do_hh_expect ~equivalent:true env pos_id targs p tys
        else if not (List.is_empty targs) then (
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Expected_tparam
                   { decl_pos = Pos_or_decl.none; pos = pos_id; n = 0 });
          env
        ) else if String.equal s SN.PseudoFunctions.hh_show then (
          List.iter tys ~f:(Typing_log.hh_show p env);
          env
        ) else if String.equal s SN.PseudoFunctions.hh_show_env then (
          Typing_log.hh_show_env p env;
          env
        ) else if String.equal s SN.PseudoFunctions.hh_log_level then
          match args with
          | [
           (Ast_defs.Pnormal, (_, _, Aast.String key_str));
           (Ast_defs.Pnormal, (_, _, Aast.Int level_str));
          ] ->
            Env.set_log_level env key_str (int_of_string level_str)
          | _ -> env
        else if String.equal s SN.PseudoFunctions.hh_force_solve then (
          let (env, ty_err_opt) = Typing_solver.solve_all_unsolved_tyvars env in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          env
        ) else if String.equal s SN.PseudoFunctions.hh_loop_forever then
          let _ = loop_forever env in
          env
        else if String.equal s SN.PseudoFunctions.hh_time then begin
          do_hh_time args;
          env
        end else
          env
      in
      (* Discard the environment and whether fake members should be forgotten to
         make sure that pseudo functions don't change the typechecker behaviour.
      *)
      let ((_env, te, ty), _should_forget_fakes) =
        let env = might_throw ~join_pos:p env in
        dispatch_call ~expected ~ctxt p env e targs args None
      in
      (env, te, ty)
    | Call { func; targs; args; unpacked_arg } ->
      let env = might_throw ~join_pos:p env in
      let ((env, te, ty), should_forget_fakes) =
        dispatch_call ~expected ~ctxt p env func targs args unpacked_arg
      in
      let env =
        if should_forget_fakes then
          Env.forget_members env Reason.(Blame (p, BScall))
        else
          env
      in
      (env, te, ty)
    | FunctionPointer (FP_id fid, targs) ->
      let (env, fty, targs) = fun_type_of_id env fid targs [] in
      let e = Aast.FunctionPointer (FP_id fid, targs) in
      let (env, fty) = set_function_pointer env fty in
      (* All function pointers are readonly_this since they are always a toplevel function or static method *)
      let (env, fty) = set_readonly_this p env fty in
      make_result env p e fty
    | Binop { bop; lhs = e1; rhs = e2 } ->
      Binop.check_binop
        ~check_defined:config.Config.check_defined
        ~expected
        env
        outer
        p
        bop
        e1
        (Either.First e2)
    | Pipe (e0, e1, e2) ->
      (* If it weren't for local variable assignment or refinement the pipe
       * expression e1 |> e2 could be typed using this rule (E is environment with
       * types for locals):
       *
       *    E |- e1 : ty1    E[$$:ty1] |- e2 : ty2
       *    --------------------------------------
       *                E |- e1|>e2 : ty2
       *
       * The possibility of e2 changing the types of locals in E means that E
       * can evolve, and so we need to restore $$ to its original state.
       *)
      let (env, te1, ty1) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e1
      in
      let dd_var = Local_id.make_unscoped SN.SpecialIdents.dollardollar in
      let dd_old_ty =
        if Env.is_local_present env dd_var then
          Some (Env.get_local env dd_var)
        else
          None
      in
      (* $$ isn't a typed local so it doesn't need a bound *)
      let env =
        Env.set_local ~is_defined:true ~bound_ty:None env dd_var ty1 Pos.none
      in
      let (env, te2, ty2) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e2
      in
      let env =
        match dd_old_ty with
        | None -> Env.unset_local env dd_var
        | Some local ->
          Typing_local_types.(
            Env.set_local
              ~is_defined:true
              ~bound_ty:local.bound_ty
              env
              dd_var
              local.ty
              local.pos)
      in
      let (env, te, ty) = make_result env p (Aast.Pipe (e0, te1, te2)) ty2 in
      (env, te, ty)
    | Unop (uop, e) ->
      let (env, te, ty) =
        raw_expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e
      in
      let env = might_throw ~join_pos:p env in
      let (env, tuop, ty) = Typing_arithmetic.unop p env uop te ty in
      let env = Typing_local_ops.check_assignment env te in
      (env, tuop, ty)
    | Eif (c, e1, e2) ->
      eif env ~expected ~in_await:ctxt.Context.in_await p c e1 e2
    | Class_const ((_, p, CI sid), pstr)
      when String.equal (snd pstr) "class" && Env.is_typedef env (snd sid) ->
    begin
      match Env.get_typedef env (snd sid) with
      | Some { td_tparams = tparaml; _ } ->
        (* Typedef type parameters cannot have constraints *)
        let params =
          List.map
            ~f:
              begin
                fun { tp_name = (p, x); _ } ->
                  (* TODO(T69551141) handle type arguments for Tgeneric *)
                  MakeType.generic (Reason.Rwitness_from_decl p) x
              end
            tparaml
        in
        let p_ = Pos_or_decl.of_raw_pos p in
        let tdef =
          mk
            ( Reason.Rwitness_from_decl p_,
              Tapply (Positioned.of_raw_positioned sid, params) )
        in
        let typename =
          mk
            ( Reason.Rwitness_from_decl p_,
              Tapply ((p_, SN.Classes.cTypename), [tdef]) )
        in
        let (env, tparams) =
          List.map_env env tparaml ~f:(fun env _tp -> Env.fresh_type env p)
        in
        let ety_env =
          {
            (empty_expand_env_with_on_error
               (Env.invalid_type_hint_assert_primary_pos_in_current_decl env))
            with
            substs = Subst.make_locl tparaml tparams;
          }
        in
        let (env, ty_err_opt1) =
          Phase.check_tparams_constraints ~use_pos:p ~ety_env env tparaml
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
        let ((env, ty_err_opt2), ty) = Phase.localize ~ety_env env typename in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
        make_result env p (Class_const ((ty, p, CI sid), pstr)) ty
      | None ->
        (* Should not expect None as we've checked whether the sid is a typedef *)
        expr_error env p outer
    end
    | Class_const (cid, mid) ->
      class_const
        env
        p
        ~is_attribute_param:ctxt.Context.is_attribute_param
        (cid, mid)
    | Class_get (((_, _, cid_) as cid), CGstring mid, Is_prop)
      when Env.FakeMembers.is_valid_static env cid_ (snd mid) ->
      let (env, local) = Env.FakeMembers.make_static env cid_ (snd mid) p in
      let local = ((), p, Lvar (p, local)) in
      let (env, _, ty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          local
      in
      let (env, _tal, te, _) = Class_id.class_expr env [] cid in
      make_result env p (Aast.Class_get (te, Aast.CGstring mid, Is_prop)) ty
    (* Statically-known static property access e.g. Foo::$x *)
    | Class_get (((_, _, cid_) as cid), CGstring mid, prop_or_method) ->
      let (env, _tal, te, cty) = Class_id.class_expr env [] cid in
      let env = might_throw ~join_pos:p env in
      let (env, (ty, _tal)) =
        Class_get_expr.class_get
          ~is_method:(equal_prop_or_method prop_or_method Is_method)
          ~is_const:false
          ~transform_fty:None
          ~coerce_from_ty:None
          env
          cty
          mid
          cid
      in
      let (env, ty) =
        Env.FakeMembers.check_static_invalid env cid_ (snd mid) ty
      in
      make_result
        env
        p
        (Aast.Class_get (te, Aast.CGstring mid, prop_or_method))
        ty
    (* Dynamic static method invocation or property access. `$y = "bar"; Foo::$y()` is interpreted as `Foo::bar()` *)
    | Class_get (cid, CGexpr m, prop_or_method) ->
      let (env, _tal, te, cty) = Class_id.class_expr env [] cid in
      (* Match Obj_get dynamic instance property access behavior *)
      let (env, tm, _) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          m
      in
      let (env, ty) =
        if
          TUtils.is_dynamic env cty
          || TCO.enable_sound_dynamic (Env.get_tcopt env)
        then
          (env, MakeType.dynamic (Reason.Rwitness p))
        else
          Env.fresh_type_error env p
      in
      let env = might_throw ~join_pos:p env in
      make_result env p (Aast.Class_get (te, Aast.CGexpr tm, prop_or_method)) ty
    (* Fake member property access. For example:
     *   if ($x->f !== null) { ...$x->f... }
     *)
    | Obj_get (e, (_, pid, Id (py, y)), nf, is_prop)
      when Env.FakeMembers.is_valid env e y ->
      let env = might_throw ~join_pos:p env in
      let (env, local) = Env.FakeMembers.make env e y p in
      let local = ((), p, Lvar (p, local)) in
      let (env, _, ty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          local
      in
      let env = Xhp_attribute.xhp_check_get_attribute p env e y nf in
      let (env, t_lhs, _) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = true }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e
      in
      let (env, t_rhs) =
        TUtils.make_simplify_typed_expr env pid ty (Aast.Id (py, y))
      in
      make_result env p (Aast.Obj_get (t_lhs, t_rhs, nf, is_prop)) ty
    (* Statically-known instance property access e.g. $x->f *)
    | Obj_get (e1, (_, pm, Id m), nullflavor, prop_or_method) ->
      let env =
        Xhp_attribute.xhp_check_get_attribute p env e1 (snd m) nullflavor
      in
      let (env, te1, ty1) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = true }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e1
      in
      let env = might_throw ~join_pos:p env in
      (* We typecheck Obj_get by checking whether it is a subtype of
         Thas_member(m, #1) where #1 is a fresh type variable. *)
      let (env, mem_ty) = Env.fresh_type env p in
      let (_, p1, _) = e1 in
      let r = Reason.Rwitness p1 in
      let has_member_ty =
        MakeType.has_member
          r
          ~name:m
          ~ty:mem_ty
          ~class_id:(CIexpr e1)
          ~explicit_targs:None
      in
      let lty1 = LoclType ty1 in
      let ((env, ty_err_opt), result_ty, ty_mismatch_opt) =
        match nullflavor with
        | OG_nullthrows ->
          let (env, ty_err_opt) =
            Type.sub_type_i
              p1
              Reason.URnone
              env
              lty1
              has_member_ty
              Typing_error.Callback.unify_error
          in
          let ty_mismatch =
            mk_ty_mismatch_opt ty1 (MakeType.nothing Reason.none) ty_err_opt
          in
          ((env, ty_err_opt), mem_ty, ty_mismatch)
        | OG_nullsafe ->
          (* In that case ty1 is a subtype of ?Thas_member(m, #1)
             and the result is ?#1 if ty1 is nullable. *)
          let r = Reason.Rnullsafe_op p in
          let null_ty = MakeType.null r in
          let (env, null_has_mem_ty) =
            Union.union_i env r has_member_ty null_ty
          in
          let (env, ty_err_opt) =
            Type.sub_type_i
              p1
              Reason.URnone
              env
              lty1
              null_has_mem_ty
              Typing_error.Callback.unify_error
          in
          let ty_mismatch =
            mk_ty_mismatch_opt ty1 (MakeType.nothing Reason.none) ty_err_opt
          in
          let (env, null_or_nothing_ty) = Inter.intersect env ~r null_ty ty1 in
          let (env, result_ty) = Union.union env null_or_nothing_ty mem_ty in
          ((env, ty_err_opt), result_ty, ty_mismatch)
      in

      let (env, result_ty) =
        Env.FakeMembers.check_instance_invalid env e1 (snd m) result_ty
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      let (env, inner_te) =
        TUtils.make_simplify_typed_expr env pm result_ty (Aast.Id m)
      in
      make_result
        env
        p
        (Aast.Obj_get
           ( hole_on_ty_mismatch ~ty_mismatch_opt te1,
             inner_te,
             nullflavor,
             prop_or_method ))
        result_ty
    (* Dynamic instance property access e.g. $x->$f *)
    | Obj_get (e1, e2, nullflavor, prop_or_method) ->
      let (env, te1, ty1) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = true }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e1
      in
      let (env, ty_err_opt) =
        (* Under Sound Dynamic, check that e1 supports dynamic *)
        Typing_coercion.coerce_type
          ~coerce:(Some Typing_logic.CoerceToDynamic)
          p
          Reason.URdynamic_prop
          env
          ty1
          (MakeType.unenforced (MakeType.dynamic (Reason.Rwitness p)))
          Typing_error.Callback.unify_error
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      let (env, te2, _) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e2
      in
      let (env, ty) = (env, MakeType.dynamic (Reason.Rdynamic_prop p)) in
      let (_, pos, te2) = te2 in
      let env = might_throw ~join_pos:p env in
      let (env, te2) = TUtils.make_simplify_typed_expr env pos ty te2 in
      make_result env p (Aast.Obj_get (te1, te2, nullflavor, prop_or_method)) ty
    | Yield af ->
      let (env, (taf, opt_key, value)) = Afield.array_field env af in
      let Typing_env_return_info.{ return_type = expected_return; _ } =
        Env.get_return env
      in
      let send =
        match get_node expected_return.et_type with
        | Tclass (_, _, _ :: _ :: send :: _) -> send
        | Tdynamic when Tast.is_under_dynamic_assumptions env.checked ->
          expected_return.et_type
        | _ ->
          Errors.internal_error p "Return type is not a generator";
          MakeType.union (Reason.Ryield_send p) []
      in
      let is_async =
        match Env.get_fn_kind env with
        | Ast_defs.FGenerator -> false
        (* This could also catch sync/async non-generators, but an error would
         * have already been generated elsewhere *)
        | _ -> true
      in
      let (env, key) =
        match (af, opt_key) with
        | (AFvalue (_, p, _), None) ->
          if is_async then
            let (env, ty) = Env.fresh_type env p in
            (env, MakeType.nullable (Reason.Ryield_asyncnull p) ty)
          else
            (env, MakeType.int (Reason.Rwitness p))
        | (_, Some x) -> (env, x)
        | (_, _) -> assert false
      in
      let rty =
        if is_async then
          MakeType.async_generator (Reason.Ryield_asyncgen p) key value send
        else
          MakeType.generator (Reason.Ryield_gen p) key value send
      in
      let Typing_env_return_info.{ return_type = expected_return; _ } =
        Env.get_return env
      in
      let (env, ty_err_opt) =
        Typing_coercion.coerce_type
          p
          Reason.URyield
          env
          rty
          expected_return
          Typing_error.Callback.unify_error
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      let env = Env.forget_members env Reason.(Blame (p, BScall)) in
      let env = LEnv.save_and_merge_next_in_cont ~join_pos:p env C.Exit in
      make_result
        env
        p
        (Aast.Yield taf)
        (MakeType.nullable (Reason.Ryield_send p) send)
    | Await e ->
      begin
        match e with
        | (_, p, Aast.Call { func = (_, _, Aast.Id (_, f)); _ })
          when String.equal f SN.PseudoFunctions.unsafe_cast
               || String.equal f SN.PseudoFunctions.unsafe_nonnull_cast ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(primary @@ Primary.Unsafe_cast_await p)
        | _ -> ()
      end;
      let env = might_throw ~join_pos:p env in
      (* Await is permitted in a using clause e.g. using (await make_handle()) *)
      let (env, te, rty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              {
                default with
                is_attribute_param = ctxt.is_attribute_param;
                is_using_clause = ctxt.is_using_clause;
                in_await = Some (Reason.Rwitness p);
              }
          env
          e
      in
      let (env, ty) = Async.overload_extract_from_awaitable env ~p rty in
      make_result env p (Aast.Await te) ty
    | ReadonlyExpr e ->
      let env = Env.set_readonly env true in
      let (env, te, rty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              {
                default with
                is_attribute_param = ctxt.is_attribute_param;
                is_using_clause = ctxt.is_using_clause;
              }
          env
          e
      in
      make_result env p (Aast.ReadonlyExpr te) rty
    | New (((_, pos, _) as cid), explicit_targs, el, unpacked_element, ()) ->
      let env = might_throw ~join_pos:p env in
      let ( env,
            tc,
            tal,
            tel,
            typed_unpack_element,
            ty,
            ctor_fty,
            should_forget_fakes ) =
        new_object
          ~expected
          ~is_using_clause:ctxt.Context.is_using_clause
          ~check_parent:false
          ~check_not_abstract:true
          pos
          env
          cid
          explicit_targs
          (List.map ~f:(fun e -> (Ast_defs.Pnormal, e)) el)
          unpacked_element
      in
      let env =
        if should_forget_fakes then
          Env.forget_members env Reason.(Blame (p, BScall))
        else
          env
      in
      make_result
        env
        p
        (Aast.New (tc, tal, List.map ~f:snd tel, typed_unpack_element, ctor_fty))
        ty
    | Cast (hint, e) ->
      let (env, te, ty2) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              {
                default with
                is_attribute_param = ctxt.is_attribute_param;
                in_await = ctxt.in_await;
              }
          env
          e
      in
      let env = might_throw ~join_pos:p env in
      let (env, ty_err_opt1) =
        if
          TCO.experimental_feature_enabled
            (Env.get_tcopt env)
            TCO.experimental_forbid_nullable_cast
          && not (TUtils.is_mixed env ty2)
        then
          SubType.sub_type_or_fail env ty2 (MakeType.nonnull (get_reason ty2))
          @@ Some
               Typing_error.(
                 primary
                 @@ Primary.Nullable_cast
                      {
                        pos = p;
                        ty_name = lazy (Typing_print.error env ty2);
                        ty_pos = get_pos ty2;
                      })
        else
          (env, None)
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
      let ((env, ty_err_opt2), ty) =
        Phase.localize_hint_no_subst env ~ignore_errors:false hint
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
      make_result env p (Aast.Cast (hint, te)) ty
    | ExpressionTree et -> Expression_tree.expression_tree env p et
    | Is (e, hint) ->
      let (env, te, _) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e
      in
      make_result env p (Aast.Is (te, hint)) (MakeType.bool (Reason.Rwitness p))
    | As { expr = e; hint; is_nullable; enforce_deep } ->
      let refine_type env lpos lty rty =
        let reason = Reason.Ras lpos in
        let (env, rty) = Env.expand_type env rty in
        refine_and_simplify_intersection
          ~hint_first:false
          env
          p
          reason
          lpos
          lty
          rty
      in
      let (env, te, expr_ty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e
      in
      let env = might_throw ~join_pos:p env in
      let ((env, ty_err_opt1), hint_ty) =
        Phase.localize_hint_for_refinement env hint
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
      let hint_ty = strip_supportdyn hint_ty in
      let hint_ty =
        if TCO.pessimise_builtins (Env.get_tcopt env) then
          ish_weakening env hint hint_ty
        else
          hint_ty
      in
      let ((env, ty_err_opt2), hint_ty) =
        if Typing_defs.is_dynamic hint_ty then
          let enable_sound_dynamic = TCO.enable_sound_dynamic env.genv.tcopt in
          let (env, ty_err_opt) =
            if enable_sound_dynamic then
              TUtils.supports_dynamic env expr_ty
              @@ Some (Typing_error.Reasons_callback.unify_error_at p)
            else
              (env, None)
          in
          let (env, locl) = make_a_local_of ~include_this:true env e in
          let env =
            match locl with
            | Some ivar ->
              let bound_ty = get_bound_ty_for_lvar env e in
              let env =
                match bound_ty with
                | None -> env
                | Some bound_ty ->
                  (* If the typed local variable will have its type replaced by dynamic,
                     we need to check that the bound isn't violated. *)
                  let (env, err) =
                    SubType.sub_type ~coerce:None env hint_ty bound_ty
                    @@ Some (Typing_error.Reasons_callback.unify_error_at p)
                  in
                  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) err;
                  env
              in
              set_local ~is_defined:true ~bound_ty env ivar hint_ty
            | None -> env
          in
          ((env, ty_err_opt), hint_ty)
        else if is_nullable then
          let (_, e_p, _) = e in
          let (env, hint_ty) = refine_type env e_p expr_ty hint_ty in
          ((env, None), MakeType.nullable (Reason.Rwitness p) hint_ty)
        else
          let (env, locl) = make_a_local_of ~include_this:true env e in
          match locl with
          | Some ((ivar_pos, _) as ivar) ->
            let (env, hint_ty) = refine_type env ivar_pos expr_ty hint_ty in
            let bound_ty = get_bound_ty_for_lvar env e in
            (* refine_type returns a subtype of the expr type, and hence cannot violate the bound *)
            let env = set_local ~is_defined:true ~bound_ty env ivar hint_ty in
            ((env, None), hint_ty)
          | None ->
            let (_, e_p, _) = e in
            let (env, ty) = refine_type env e_p expr_ty hint_ty in
            ((env, None), ty)
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
      make_result
        env
        p
        (Aast.As { expr = te; hint; is_nullable; enforce_deep })
        hint_ty
    | Upcast (e, hint) ->
      let (env, te, expr_ty) =
        expr
          ~expected:None
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          e
      in
      let ((env, ty_err_opt), hint_ty) =
        Phase.localize_hint_no_subst env ~ignore_errors:false hint
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      let env = upcast env p expr_ty hint_ty in
      make_result env p (Aast.Upcast (te, hint)) hint_ty
    | Efun
        { ef_fun = f; ef_use = idl; ef_closure_class_name = closure_class_name }
      ->
      Lambda.lambda ~is_anon:true ~closure_class_name ~expected p env f idl
    | Lfun (f, idl) ->
      Lambda.lambda
        ~is_anon:false
        ~closure_class_name:None
        ~expected
        p
        env
        f
        idl
    | Xml (sid, attrl, el) ->
      let cid = CI sid in
      let (env, _tal, _te, classes) =
        Class_id.class_id_for_new ~exact:nonexact p env cid []
      in
      (* OK to ignore rest of list; class_info only used for errors, and
         * cid = CI sid cannot produce a union of classes anyhow *)
      let class_info =
        List.find_map classes ~f:(function
            | `Dynamic -> None
            | `Class (_, class_info, _) -> Some class_info)
      in
      let (env, te, obj) =
        (* New statements derived from Xml literals are of the following form:
         *
         *   __construct(
         *     darray<string,mixed> $attributes,
         *     varray<mixed> $children,
         *     string $file,
         *     int $line
         *   );
         *)
        let new_exp = Typing_xhp.rewrite_xml_into_new p sid attrl el in
        expr
          ~expected
          ~config:Config.{ config with accept_using_var = false }
          ~ctxt:
            Context.
              { default with is_attribute_param = ctxt.is_attribute_param }
          env
          new_exp
      in

      let tchildren =
        match te with
        | ( _,
            _,
            New
              ( _,
                _,
                [
                  _;
                  ( _,
                    _,
                    ( Varray (_, children)
                    | ValCollection ((_, Vec), _, children) ) );
                  _;
                  _;
                ],
                _,
                _ ) ) ->
          (* Typing_xhp.rewrite_xml_into_new generates an AST node for a `varray[]` literal, which is interpreted as a vec[]  *)
          children
        | _ ->
          (* We end up in this case when the cosntructed new expression does
             not typecheck. *)
          []
      in
      let (env, typed_attrs) =
        Xhp_attribute.xhp_attribute_exprs env class_info attrl sid obj
      in
      let txml = Aast.Xml (sid, typed_attrs, tchildren) in
      (match class_info with
      | None ->
        let ty = MakeType.nothing (Reason.Rmissing_class p) in
        make_result env p txml ty
      | Some _ -> make_result env p txml obj)
    | Shape fdm ->
      let expr_helper ?expected env (k, e) =
        let (env, et, ty) =
          expr
            ~expected
            ~config:Config.{ config with accept_using_var = false }
            ~ctxt:
              Context.
                { default with is_attribute_param = ctxt.is_attribute_param }
            env
            e
        in
        (env, (k, et, ty))
      in
      let (env, tfdm) =
        match
          expand_expected_and_get_node
            ~pessimisable_builtin:false
            ~strip_supportdyn:true
            env
            expected
        with
        | (env, Some (pos, ur, _, _, Tshape { s_fields = expected_fdm; _ })) ->
          List.map_env
            env
            ~f:(fun env ((k, _) as ke) ->
              let tk = TShapeField.of_ast Pos_or_decl.of_raw_pos k in
              match TShapeMap.find_opt tk expected_fdm with
              | None -> expr_helper env ke
              | Some sft ->
                expr_helper ~expected:(ExpectedTy.make pos ur sft.sft_ty) env ke)
            fdm
        | _ -> List.map_env env ~f:expr_helper fdm
      in
      let fdm =
        List.fold_left
          ~f:(fun acc (k, _, ty) ->
            let tk = TShapeField.of_ast Pos_or_decl.of_raw_pos k in
            TShapeMap.add tk { sft_optional = false; sft_ty = ty } acc)
          ~init:TShapeMap.empty
          tfdm
      in
      let env =
        Typing_shapes.check_shape_keys_validity env (List.map tfdm ~f:fst3)
      in
      (* Fields are fully known, because this shape is constructed
       * using shape keyword and we know exactly what fields are set. *)
      make_result
        env
        p
        (Aast.Shape (List.map ~f:(fun (k, te, _) -> (k, te)) tfdm))
        (MakeType.closed_shape (Reason.Rshape_literal p) fdm)
    | ET_Splice e ->
      Env.with_outside_expr_tree env (fun env dsl_name ->
          et_splice env dsl_name p e)
    | EnumClassLabel (None, s) ->
      let (env, expect_label, lty_opt) =
        match expected with
        | Some ety ->
          let (env, lty) = Env.expand_type env ety.ExpectedTy.ty.et_type in
          let expect_label =
            match get_node (TUtils.strip_dynamic env lty) with
            | Tnewtype (name, _, _) ->
              String.equal SN.Classes.cEnumClassLabel name
            | _ -> false
          in
          (env, expect_label, Some lty)
        | None -> (env, false, None)
      in
      let () =
        if expect_label then
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(enum @@ Primary.Enum.Enum_class_label_as_expr p)
        else
          let expected_ty_msg_opt =
            Option.map lty_opt ~f:(fun lty ->
                let ty_str = lazy (Typing_print.error env lty) in
                let r = get_reason lty in
                Lazy.map ty_str ~f:(fun ty_str ->
                    Reason.to_string (Format.sprintf "Expected %s" ty_str) r))
          in
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              enum
              @@ Primary.Enum.Enum_class_label_member_mismatch
                   { pos = p; label = s; expected_ty_msg_opt })
      in
      let (env, ty) = Env.fresh_type_error env p in
      make_result env p (Aast.EnumClassLabel (None, s)) ty
    | EnumClassLabel ((Some cname as e), name) ->
      let (env, res) =
        EnumClassLabelOps.expand
          p
          env
          ~full:true
          ~ctor:SN.Classes.cEnumClassLabel
          cname
          name
          None
      in
      let error () =
        let (env, ty) = Env.fresh_type_error env p in
        make_result env p (Aast.EnumClassLabel (e, name)) ty
      in
      (match res with
      | EnumClassLabelOps.Success ((_, _, texpr), lty) ->
        make_result env p texpr lty
      | EnumClassLabelOps.ClassNotFound ->
        (* Error registered in nast_check/unbound_name_check *)
        error ()
      | EnumClassLabelOps.LabelNotFound _ ->
        (* Error registered in EnumClassLabelOps.expand *)
        error ()
      | EnumClassLabelOps.Skip ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(enum @@ Primary.Enum.Enum_class_label_as_expr p);
        error ())
    | Package ((p, _) as id) ->
      make_result env p (Aast.Package id) (MakeType.bool (Reason.Rwitness p))
    | Nameof cid ->
      let (env, _tal, ce, cty) = Class_id.class_expr env [] cid in
      make_result
        env
        p
        (Aast.Nameof ce)
        (MakeType.classname (Reason.Rwitness p) [cty])

  and class_const
      ?(is_attribute_param = false) ?(incl_tc = false) env p (cid, mid) =
    let (env, _tal, ce, cty) =
      Class_id.class_expr ~is_attribute_param env [] cid
    in
    let env =
      match get_node cty with
      | Tclass ((_, n), _, _)
        when Env.is_enum_class env n && String.(SN.Members.mClass <> snd mid) ->
        Typing_local_ops.enforce_enum_class_variant p env
      | _ -> env
    in
    let (env, (const_ty, _tal)) =
      Class_get_expr.class_get
        ~is_method:false
        ~is_const:true
        ~transform_fty:None
        ~incl_tc
        ~coerce_from_ty:None
        env
        cty
        mid
        cid
    in
    make_result env p (Aast.Class_const (ce, mid)) const_ty

  and et_splice env dsl_opt p e =
    let open Option.Let_syntax in
    let dsl_opt =
      let* (_, dsl_name) = dsl_opt in
      return dsl_name
    in
    (* Error Reporting:
       The unification error reported by checking against `Spliceable<_, _, _>`
       is confusing to users. We improve the error message by including
       contextual information we pull from the definition of the DSL class.

       Particularly we:
         - Use the type specified in the DSL classes splice method
         - If there is a docs url attached to the class, use it *)
    let rec contextual_reasons ~ty env =
      let* dsl_name = dsl_opt in
      let* cls = Env.get_class env dsl_name in
      let* { ce_type = (lazy fun_ty); _ } =
        Env.get_member true env cls SN.ExpressionTrees.splice
      in
      (* Pull the type of third argument to `MyDsl::splice()`.
         If we change the number of arguments this method takes, we will need to
         update this match expression *)
      let* splice_ty =
        match get_node fun_ty with
        | Tfun { ft_params = [_; _; splice_ty]; _ } ->
          return @@ splice_ty.fp_type.et_type
        | _ -> None
      in
      return @@ lazy (error_reason ~splice_ty ~ty ~dsl_name env)
    and error_reason ~splice_ty ~ty ~dsl_name env =
      let err_splice_ty =
        Printf.sprintf
          "Expected `%s` because you are splicing into a `%s` expression or block"
          (Typing_print.with_blank_tyvars (fun () ->
               Typing_print.full_strip_ns_decl env splice_ty))
          (Utils.strip_ns dsl_name)
      in
      let err_ty =
        Printf.sprintf "But got `%s`"
        @@ Typing_print.with_blank_tyvars (fun () ->
               Typing_print.full_strip_ns env ty)
      in
      Reason.to_string err_splice_ty (get_reason splice_ty)
      @ Reason.to_string err_ty (get_reason ty)
    in
    let docs_url =
      lazy
        (let* dsl_name = dsl_opt in
         let* cls = Env.get_class env dsl_name in
         Cls.get_docs_url cls)
    in

    let (env, te, ty) =
      expr ~expected:None ~config:Config.default ~ctxt:Context.default env e
    in
    let (env, ty_visitor) = Env.fresh_type env p in
    let (env, ty_res) = Env.fresh_type env p in
    let (env, ty_infer) = Env.fresh_type env p in
    let spliceable_type =
      let raw_spliceable_type =
        MakeType.spliceable (Reason.Rsplice p) ty_visitor ty_res ty_infer
      in
      if TCO.pessimise_builtins (Env.get_tcopt env) then
        MakeType.locl_like (Reason.Rsplice p) raw_spliceable_type
      else
        raw_spliceable_type
    in

    let (_, expr_pos, _) = e in
    let (env, ty_err_opt) =
      SubType.sub_type env ty spliceable_type
      @@ Some
           (Typing_error.Reasons_callback.expr_tree_splice_error
              p
              ~expr_pos:(Pos_or_decl.of_raw_pos expr_pos)
              ~contextual_reasons:(contextual_reasons ~ty env)
              ~dsl_opt
              ~docs_url)
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    make_result env p (Aast.ET_Splice te) ty_infer

  and new_object
      ~(expected : ExpectedTy.t option)
      ~check_parent
      ~check_not_abstract
      ~is_using_clause
      p
      env
      ((_, _, cid_) as cid)
      explicit_targs
      el
      unpacked_element =
    (* Obtain class info from the cid expression. We get multiple
     * results with a CIexpr that has a union type, e.g. in

      $classname = (mycond()? classname<A>: classname<B>);
      new $classname();
     *)
    let (env, tal, tcid, classes) =
      Class_id.instantiable_cid ~exact:Exact p env cid_ explicit_targs
    in
    let allow_abstract_bound_generic =
      match tcid with
      | (ty, _, Aast.CI (_, tn)) -> is_generic_equal_to tn ty
      | _ -> false
    in
    let gather
        (env, _tel, _typed_unpack_element, should_forget_fakes_acc)
        (cname, class_info, c_ty) =
      if
        check_not_abstract
        && Cls.abstract class_info
        && (not (requires_consistent_construct cid_))
        && not allow_abstract_bound_generic
      then
        uninstantiable_error
          env
          p
          cid_
          (Cls.pos class_info)
          (Cls.name class_info)
          p
          c_ty;
      let (env, obj_ty_, params) =
        let (env, c_ty) = Env.expand_type env c_ty in
        match (cid_, tal, get_class_type c_ty) with
        (* Explicit type arguments *)
        | (CI _, _ :: _, Some (_, _, tyl)) -> (env, get_node c_ty, tyl)
        | (_, _, class_type_opt) ->
          let (env, params) =
            List.map_env env (Cls.tparams class_info) ~f:(fun env tparam ->
                let (env, tvar) =
                  Env.fresh_type_reason
                    env
                    p
                    (Reason.Rtype_variable_generics
                       (p, snd tparam.tp_name, strip_ns (snd cname)))
                in
                Typing_log.log_new_tvar_for_new_object env p tvar cname tparam;
                (env, tvar))
          in
          begin
            match class_type_opt with
            | Some (_, Exact, _) -> (env, Tclass (cname, Exact, params), params)
            | _ -> (env, Tclass (cname, nonexact, params), params)
          end
      in
      if
        (not check_parent)
        && (not is_using_clause)
        && Typing_disposable.is_disposable_class env class_info
      then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.Invalid_new_disposable p);
      let r_witness = Reason.Rwitness p in
      let obj_ty = mk (r_witness, obj_ty_) in
      let (env, c_ty) =
        match cid_ with
        | CIstatic -> (env, mk (r_witness, get_node c_ty))
        | CIexpr _ ->
          let c_ty = mk (r_witness, get_node c_ty) in
          (* When constructing from a (classname) variable, the variable
           * dictates what the constructed object is going to be. This allows
           * for generic and dependent types to be correctly carried
           * through the 'new $foo()' iff the constructed obj_ty is a
           * supertype of the variable-dictated c_ty *)
          let (env, ty_err_opt) =
            Typing_ops.sub_type
              p
              Reason.URnone
              env
              c_ty
              obj_ty
              Typing_error.Callback.unify_error
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          (env, c_ty)
        | _ -> (env, obj_ty)
      in
      let (env, new_ty) =
        let (cid_ty, _, _) = tcid in
        let (env, cid_ty) = Env.expand_type env cid_ty in
        if is_generic cid_ty then
          (env, cid_ty)
        else if check_parent then
          (env, c_ty)
        else
          ExprDepTy.make env ~cid:cid_ c_ty
      in
      (* Set variance according to type of `new` expression now. Lambda arguments
       * to the constructor might depend on it, and `call_construct` only uses
       * `ctor_fty` to set the variance which has void return type *)
      let env = Env.set_tyvar_variance env new_ty in
      let (env, tel, typed_unpack_element, ctor_fty, should_forget_fakes) =
        let env = check_expected_ty "New" env new_ty expected in
        call_construct p env class_info params el unpacked_element cid new_ty
      in
      let should_forget_fakes_acc =
        should_forget_fakes_acc || should_forget_fakes
      in
      (if equal_consistent_kind (snd (Cls.construct class_info)) Inconsistent
      then
        match cid_ with
        | CIstatic ->
          let (class_pos, class_name) = cname in
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.New_inconsistent_construct
                   { pos = p; class_name; class_pos; kind = `static })
        | CIexpr _ ->
          let (class_pos, class_name) = cname in
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.New_inconsistent_construct
                   { pos = p; class_name; class_pos; kind = `classname })
        | _ -> ());
      match cid_ with
      | CIparent ->
        let (env, ctor_fty) =
          match fst (Cls.construct class_info) with
          | Some ({ ce_type = (lazy ty); _ } as ce) ->
            let ety_env =
              {
                empty_expand_env with
                substs =
                  TUtils.make_locl_subst_for_class_tparams class_info params;
                this_ty = obj_ty;
              }
            in
            if get_ce_abstract ce then
              Typing_error_utils.add_typing_error
                ~env
                Typing_error.(
                  primary
                  @@ Primary.Parent_abstract_call
                       {
                         meth_name = SN.Members.__construct;
                         pos = p;
                         decl_pos = get_pos ctor_fty;
                       });
            let ((env, ty_err_opt), ctor_fty) =
              Phase.localize ~ety_env env ty
            in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            (env, ctor_fty)
          | None -> (env, ctor_fty)
        in
        ( (env, tel, typed_unpack_element, should_forget_fakes_acc),
          (obj_ty, ctor_fty) )
      | CIstatic
      | CI _
      | CIself ->
        ( (env, tel, typed_unpack_element, should_forget_fakes_acc),
          (c_ty, ctor_fty) )
      | CIexpr _ ->
        ( (env, tel, typed_unpack_element, should_forget_fakes_acc),
          (c_ty, ctor_fty) )
    in
    let (had_dynamic, classes) =
      List.fold classes ~init:(false, []) ~f:(fun (seen_dynamic, classes) -> function
        | `Dynamic -> (true, classes)
        | `Class (cname, class_info, c_ty) ->
          (seen_dynamic, (cname, class_info, c_ty) :: classes))
    in
    let ( (env, tel, typed_unpack_element, should_forget_fakes),
          class_types_and_ctor_types ) =
      List.fold_map classes ~init:(env, [], None, false) ~f:gather
    in
    let class_types_and_ctor_types =
      let r = Reason.Rdynamic_construct p in
      let dyn = (mk (r, Tdynamic), mk (r, Tdynamic)) in
      if had_dynamic then
        dyn :: class_types_and_ctor_types
      else
        class_types_and_ctor_types
    in
    let (env, tel, typed_unpack_element, ty, ctor_fty) =
      match class_types_and_ctor_types with
      | [] ->
        let (env, tel, _) =
          argument_list_exprs
            (expr ~expected:None ~config:Config.default ~ctxt:Context.default)
            env
            el
        in
        let (env, typed_unpack_element, _) =
          match unpacked_element with
          | None -> (env, None, MakeType.nothing Reason.Rnone)
          | Some unpacked_element ->
            let (env, e, ty) =
              expr
                ~expected:None
                ~config:Config.default
                ~ctxt:Context.default
                env
                unpacked_element
            in
            (env, Some e, ty)
        in
        let (env, ty) = Env.fresh_type_error env p in
        (env, tel, typed_unpack_element, ty, ty)
      | [(ty, ctor_fty)] -> (env, tel, typed_unpack_element, ty, ctor_fty)
      | l ->
        let (tyl, ctyl) = List.unzip l in
        let r = Reason.Rwitness p in
        (env, tel, typed_unpack_element, mk (r, Tunion tyl), mk (r, Tunion ctyl))
    in
    let (env, new_ty) =
      let (cid_ty, _, _) = tcid in
      let (env, cid_ty) = Env.expand_type env cid_ty in
      if is_generic cid_ty then
        (env, cid_ty)
      else if check_parent then
        (env, ty)
      else
        ExprDepTy.make env ~cid:cid_ ty
    in
    ( env,
      tcid,
      tal,
      tel,
      typed_unpack_element,
      new_ty,
      ctor_fty,
      should_forget_fakes )

  and array_value ~(expected : ExpectedTy.t option) env x =
    let (env, te, ty) =
      expr ~expected ~config:Config.default ~ctxt:Context.default env x
    in
    (env, (te, ty))

  and arraykey_value
      p
      class_name
      is_set
      ~(expected : ExpectedTy.t option)
      env
      ((_, pos, _) as x) =
    let (env, (te, ty)) = array_value ~expected env x in
    let (ty_arraykey, reason) =
      if is_set then
        ( MakeType.arraykey (Reason.Rset_element pos),
          Reason.set_element class_name )
      else
        (MakeType.arraykey (Reason.Ridx_dict pos), Reason.index_class class_name)
    in
    let ty_expected = { et_type = ty_arraykey; et_enforced = Enforced } in
    let ((env, ty_err_opt), te) =
      (* If we have an error in coercion here, we will add a `Hole` indicating the
           actual and expected type. The `Hole` may then be used in a codemod to
           add a call to `UNSAFE_CAST` so we need to consider what type we expect.

           If we were to add an expected type of 'arraykey' here it would be
           correct but adding an `UNSAFE_CAST<?string,arraykey>($x)` means we
           get cascading errors if we have e.g. a return type of keyset<string>.

           To try and prevent this, if this is an optional type where the nonnull
           part can be coerced to arraykey, we prefer that type as our expected type.
      *)
      let (ty_actual, is_option) =
        match deref ty with
        | (_, Toption ty_inner) -> (ty_inner, true)
        | _ -> (ty, false)
      in
      let (env, e1) =
        Typing_coercion.coerce_type
          ~coerce_for_op:true
          p
          reason
          env
          ty_actual
          ty_expected
          Typing_error.Callback.unify_error
      in
      let (ty_mismatch_opt, e2) =
        match e1 with
        | None when is_option ->
          let ty_str = lazy (Typing_print.full_strip_ns env ty_actual) in
          let reasons_opt =
            Some
              (Lazy.map ty_str ~f:(fun ty_str ->
                   Reason.to_string "Expected `arraykey`" (Reason.Ridx_dict pos)
                   @ [(get_pos ty, Format.sprintf "But got `?%s`" ty_str)]))
          in
          (* We actually failed so generate the error we should
             have seen *)
          let ty_err =
            Typing_error.(
              primary
              @@ Primary.Unify_error
                   {
                     pos;
                     msg_opt = Some (Reason.string_of_ureason reason);
                     reasons_opt;
                   })
          in
          (Some (ty, ty_actual), Some ty_err)
        | Some _
        | None ->
          (None, None)
      in
      let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
      ((env, ty_err_opt), hole_on_ty_mismatch ~ty_mismatch_opt te)
    in

    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);

    (env, (te, ty))

  and check_parent_construct pos recv_pos env el unpacked_element env_parent =
    let check_not_abstract = false in
    let ((env, ty_err_opt), env_parent) =
      Phase.localize_no_subst env ~ignore_errors:true env_parent
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    let ( env,
          _tcid,
          _tal,
          tel,
          typed_unpack_element,
          parent,
          fty,
          should_forget_fakes ) =
      new_object
        ~expected:None
        ~check_parent:true
        ~check_not_abstract
        ~is_using_clause:false
        pos
        env
        ((), recv_pos, CIparent)
        []
        el
        unpacked_element
    in
    (* Not sure why we need to equate these types *)
    let (env, e1) =
      Type.sub_type
        pos
        Reason.URnone
        env
        env_parent
        parent
        Typing_error.Callback.unify_error
    in
    let (env, e2) =
      Type.sub_type
        pos
        Reason.URnone
        env
        parent
        env_parent
        Typing_error.Callback.unify_error
    in
    Option.(
      iter ~f:(Typing_error_utils.add_typing_error ~env)
      @@ merge e1 e2 ~f:Typing_error.both);
    ( env,
      tel,
      typed_unpack_element,
      MakeType.void (Reason.Rwitness pos),
      parent,
      fty,
      should_forget_fakes )

  and call_parent_construct pos recv_pos env el unpacked_element =
    match Env.get_parent_ty env with
    | Some parent ->
      check_parent_construct pos recv_pos env el unpacked_element parent
    | None ->
      (* continue here *)
      let (env, ty) = Env.fresh_type_error env pos in
      let should_invalidate_fake_members = true in
      let default =
        (env, [], None, ty, ty, ty, should_invalidate_fake_members)
      in
      (match Env.get_self_id env with
      | Some self ->
        (match Env.get_class env self with
        | Some trait when Ast_defs.is_c_trait (Cls.kind trait) ->
          (match trait_most_concrete_req_class trait env with
          | None ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(primary @@ Primary.Parent_in_trait pos);
            default
          | Some (c, parent_ty) ->
            (match Cls.construct c with
            | (_, Inconsistent) ->
              Typing_error_utils.add_typing_error
                ~env
                Typing_error.(
                  primary
                  @@ Primary.Trait_parent_construct_inconsistent
                       { pos; decl_pos = Cls.pos c })
            | _ -> ());
            check_parent_construct
              pos
              recv_pos
              env
              el
              unpacked_element
              parent_ty)
        | Some _self_tc ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(primary @@ Primary.Undefined_parent pos);
          default
        | None -> assert false)
      | None ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.Parent_outside_class pos);
        let (env, ty) = Env.fresh_type_error env pos in
        (env, [], None, ty, ty, ty, should_invalidate_fake_members))

  (* Depending on the kind of expression we are dealing with
   * The typing of call is different.
   *)
  and dispatch_call
      ~(expected : ExpectedTy.t option)
      ~ctxt
      p
      env
      ((_, fpos, fun_expr) as e : Nast.expr)
      explicit_targs
      el
      unpacked_element =
    let make_call env func targs args unpacked_arg ty =
      make_result env p (Aast.Call { func; targs; args; unpacked_arg }) ty
    in
    (* For special functions and pseudofunctions with a definition in an HHI
     * file.
     *)
    let make_call_special_from_def env id tel ty_ =
      let (env, fty, tal) = fun_type_of_id env id explicit_targs el in
      let ty =
        match get_node fty with
        | Tfun ft -> ft.ft_ret.et_type
        | _ -> ty_ (Reason.Rwitness p)
      in
      let (env, te) =
        TUtils.make_simplify_typed_expr env fpos fty (Aast.Id id)
      in
      make_call env te tal tel None ty
    in
    let overload_function = overload_function make_call fpos in
    (* Require [get_idisposable_value()] function calls to be inside a [using]
       statement. *)
    let check_disposable_in_return env fty =
      if
        is_return_disposable_fun_type env fty
        && not ctxt.Context.is_using_clause
      then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.Invalid_new_disposable p)
    in

    let dispatch_id env id =
      let (env, fty, tal) = fun_type_of_id env id explicit_targs el in
      check_disposable_in_return env fty;
      let (env, (tel, typed_unpack_element, ty, should_forget_fakes)) =
        call
          ~expected
          ~expr_pos:p
          ~recv_pos:(fst id)
          ~in_await:None
          env
          fty
          el
          unpacked_element
      in
      let (env, inner_te) =
        TUtils.make_simplify_typed_expr env fpos fty (Aast.Id id)
      in
      let result = make_call env inner_te tal tel typed_unpack_element ty in
      (result, should_forget_fakes)
    in
    (* The optional transform_fty parameter is used to transform function
     * type decls for special static methods, currently just Shapes::at
     * and Shapes::idx
     *)
    let dispatch_class_const ?transform_fty env ((_, pos, e1_) as e1) m =
      let (env, _tal, tcid, ty1) = Class_id.class_expr env [] e1 in
      let this_ty = MakeType.this (Reason.Rwitness fpos) in
      (* In static context, you can only call parent::foo() on static methods.
       * In instance context, you can call parent:foo() on static
       * methods as well as instance methods
       *)
      let is_static =
        (not (Nast.equal_class_id_ e1_ CIparent))
        || Env.is_static env
        || class_contains_smethod env ty1 m
      in
      let ((env, ty_err_opt), (fty, tal)) =
        if not is_static then
          (* parent::nonStaticFunc() is really weird. It's calling a method
           * defined on the parent class, but $this is still the child class.
           *)
          TOG.obj_get
            ~inst_meth:false
            ~meth_caller:false
            ~is_method:true
            ~nullsafe:None
            ~obj_pos:pos
            ~coerce_from_ty:None
            ~explicit_targs:[]
            ~class_id:e1_
            ~member_id:m
            ~on_error:Typing_error.Callback.unify_error
            ~parent_ty:ty1
            env
            this_ty
        else
          let (env, res) =
            Class_get_expr.class_get
              ~coerce_from_ty:None
              ~is_method:true
              ~transform_fty
              ~is_const:false
              ~explicit_targs
              env
              ty1
              m
              e1
          in
          ((env, None), res)
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      check_disposable_in_return env fty;
      let (env, (tel, typed_unpack_element, ty, should_forget_fakes)) =
        call
          ~expected
          ~expr_pos:p
          ~recv_pos:fpos
          ~in_await:None
          env
          fty
          el
          unpacked_element
      in
      let (env, te) =
        TUtils.make_simplify_typed_expr
          env
          fpos
          fty
          (Aast.Class_const (tcid, m))
      in
      let result = make_call env te tal tel typed_unpack_element ty in
      (result, should_forget_fakes)
    in
    match fun_expr with
    (* Special top-level function *)
    | Id ((pos, x) as id) when SN.StdlibFunctions.needs_special_dispatch x ->
    begin
      match x with
      (* `unsafe_cast` *)
      | unsafe_cast
        when String.equal unsafe_cast SN.PseudoFunctions.unsafe_cast
             || String.equal unsafe_cast SN.PseudoFunctions.unsafe_nonnull_cast
        ->
        let result =
          match el with
          | [(Ast_defs.Pnormal, original_expr)]
            when TCO.ignore_unsafe_cast (Env.get_tcopt env) ->
            expr
              ~expected:None
              ~config:Config.default
              ~ctxt:Context.default
              env
              original_expr
          | _ ->
            (* first type the `unsafe_cast` as a call, handling arity errors *)
            let (env, fty, tal) = fun_type_of_id env id explicit_targs el in
            check_disposable_in_return env fty;
            let (env, (tel, _, ty, _should_forget_fakes)) =
              call
                ~expected
                ~expr_pos:p
                ~recv_pos:fpos
                ~in_await:None
                env
                fty
                el
                unpacked_element
            in
            (* construct the `Hole` using default value and type arguments
               if necessary *)
            let (env, dflt_ty) = Env.fresh_type_invariant env p in
            let el =
              match tel with
              | (_, e) :: _ -> e
              | [] -> Tast.make_typed_expr fpos dflt_ty Aast.Null
            and (ty_from, ty_to) =
              match tal with
              | (ty_from, _) :: (ty_to, _) :: _ -> (ty_from, ty_to)
              | (ty, _) :: _ -> (ty, ty)
              | _ -> (dflt_ty, dflt_ty)
            in
            let hole_source =
              if String.equal unsafe_cast SN.PseudoFunctions.unsafe_cast then
                UnsafeCast (List.map ~f:snd explicit_targs)
              else
                UnsafeNonnullCast
            in
            let te = Aast.Hole (el, ty_from, ty_to, hole_source) in
            let ty = Typing_defs.with_reason ty (Reason.Runsafe_cast p) in
            make_result env p te ty
        in
        let should_forget_fakes = false in
        (result, should_forget_fakes)
        (* Special function `isset` *)
      | isset when String.equal isset SN.PseudoFunctions.isset ->
        let (env, tel, _) =
          argument_list_exprs
            (expr
               ~expected:None
               ~config:Config.{ accept_using_var = true; check_defined = false }
               ~ctxt:Context.default)
            env
            el
        in
        if Option.is_some unpacked_element then
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Unpacking_disallowed_builtin_function
                   { pos = p; fn_name = isset });
        let should_forget_fakes = false in
        let result = make_call_special_from_def env id tel MakeType.bool in
        (result, should_forget_fakes)
      (* Special function `unset` *)
      | unset when String.equal unset SN.PseudoFunctions.unset ->
        let (env, tel, _) =
          argument_list_exprs
            (expr ~expected:None ~config:Config.default ~ctxt:Context.default)
            env
            el
        in
        if Option.is_some unpacked_element then
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(
              primary
              @@ Primary.Unpacking_disallowed_builtin_function
                   { pos = p; fn_name = unset });
        let env = Typing_local_ops.check_unset_target env tel in
        let (env, ty_err_opt) =
          match (el, unpacked_element) with
          | ([(Ast_defs.Pnormal, (_, _, Array_get (ea, Some _)))], None) ->
            let (env, _te, ty) =
              expr
                ~expected:None
                ~config:Config.default
                ~ctxt:Context.default
                env
                ea
            in
            let r = Reason.Rwitness p in
            let tmixed = MakeType.mixed r in
            let super =
              mk
                ( r,
                  Tunion
                    [
                      MakeType.dynamic r;
                      MakeType.dict r tmixed tmixed;
                      MakeType.keyset r tmixed;
                    ] )
            in
            let reason =
              lazy
                (Reason.to_string
                   ("This is " ^ Typing_print.error ~ignore_dynamic:true env ty)
                   (get_reason ty))
            in
            SubType.sub_type_or_fail env ty super
            @@ Some
                 Typing_error.(
                   primary @@ Primary.Unset_nonidx_in_strict { pos = p; reason })
          | _ ->
            let ty_err =
              Typing_error.(
                primary
                @@ Primary.Unset_nonidx_in_strict { pos = p; reason = lazy [] })
            in
            (env, Some ty_err)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        let should_forget_fakes = false in
        let result =
          match el with
          | [(_, (_, p, Obj_get (_, _, OG_nullsafe, _)))] ->
            (Typing_error_utils.add_typing_error ~env
            @@ Typing_error.(
                 primary @@ Primary.Nullsafe_property_write_context p));
            let (env, ty) = Env.fresh_type_error env p in
            make_call_special_from_def env id tel (fun _ -> ty)
          | _ -> make_call_special_from_def env id tel MakeType.void
        in
        (result, should_forget_fakes)
      | type_structure
        when String.equal type_structure SN.StdlibFunctions.type_structure
             && Int.equal (List.length el) 2
             && Option.is_none unpacked_element ->
        let should_forget_fakes = false in
        (match el with
        | [e1; e2] ->
          (match e2 with
          | (_, (_, p, String cst)) ->
            (* find the class constant implicitly defined by the typeconst *)
            let cid =
              match e1 with
              | (_, (_, _, Class_const (cid, (_, x))))
              | (_, (_, _, Class_get (cid, CGstring (_, x), _)))
                when String.equal x SN.Members.mClass ->
                cid
              | _ ->
                let (_, ((_, p1, _) as e1_)) = e1 in
                ((), p1, CIexpr e1_)
            in
            let result = class_const ~incl_tc:true env p (cid, (p, cst)) in
            let () =
              match result with
              | (_, (ty, _, _), _)
                when TUtils.is_tyvar_error env ty || is_dynamic ty ->
                Typing_error_utils.add_typing_error
                  ~env
                  Typing_error.(
                    primary
                    @@ Primary.Illegal_type_structure
                         { pos; msg = "Could not resolve the type constant" })
              | _ -> ()
            in
            (result, should_forget_fakes)
          | _ ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(
                primary
                @@ Primary.Illegal_type_structure
                     { pos; msg = "Second argument is not a string" });
            let result = expr_error env pos e in
            (result, should_forget_fakes))
        | _ -> assert false)
      | _ -> dispatch_id env id
    end
    (* Special Shapes:: function *)
    | Class_const (((_, _, CI (_, shapes)) as class_id), ((_, x) as method_id))
      when String.equal shapes SN.Shapes.cShapes
           || String.equal shapes SN.Shapes.cReadonlyShapes -> begin
      match x with
      (* Special functions `Shapes::at`, `Shapes::idx` and `Shapes::keyExists`.
       * We implement these by transforming the function type decl that's fetched
       * from the hhi
       *)
      | _
        when String.equal x SN.Shapes.at
             || String.equal x SN.Shapes.idx
             || String.equal x SN.Shapes.keyExists ->
        let transform_fty =
          (* We expect the second argument to at, idx and keyExists to be a literal field name *)
          match el with
          | _ :: (_, field) :: _ -> begin
            let (fld_opt, ty_err_opt) =
              TUtils.shape_field_name_with_ty_err env field
            in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            match fld_opt with
            | None -> None
            | Some field_name ->
              let field_name =
                TShapeField.of_ast Pos_or_decl.of_raw_pos field_name
              in
              Some
                (fun fty ->
                  Typing_shapes.transform_special_shapes_fun_ty
                    field_name
                    method_id
                    (List.length el)
                    fty)
          end
          | _ -> None
        in
        dispatch_class_const env class_id method_id ?transform_fty
      (* Special function `Shapes::removeKey` *)
      | remove_key when String.equal remove_key SN.Shapes.removeKey ->
        overload_function
          p
          env
          class_id
          method_id
          el
          unpacked_element
          (fun env _ res el _tel ->
            match el with
            | [(Ast_defs.Pinout _, shape); (_, field)] -> begin
              match shape with
              | (_, _, Lvar (_, lvar))
              | (_, _, Hole ((_, _, Lvar (_, lvar)), _, _, _)) ->
                (* We reevaluate the shape instead of using the shape type
                   evaluated during typechecking against the HHI signature
                   because this argument is inout and bidirectional
                   typechecking causes a union with the open shape type
                   which defeats the purpose of this extra-logical function.
                *)
                let (env, _te, shape_ty) =
                  expr
                    ~expected:None
                    ~config:Config.default
                    ~ctxt:Context.default
                    env
                    shape
                in
                let (env, shape_ty) =
                  Typing_shapes.remove_key p env shape_ty field
                in
                let env =
                  set_valid_rvalue ~is_defined:true p env lvar None shape_ty
                in
                (env, res)
              | (_, shape_pos, _) ->
                Typing_error_utils.add_typing_error
                  ~env
                  Typing_error.(
                    shape @@ Primary.Shape.Invalid_shape_remove_key shape_pos);
                (env, res)
            end
            | _ -> (env, res))
      (* Special functions `Shapes::toDict` and `Shapes::toArray` *)
      | to_dict
        when String.equal to_dict SN.Shapes.toDict
             || String.equal to_dict SN.Shapes.toArray ->
        overload_function
          p
          env
          class_id
          method_id
          el
          unpacked_element
          (fun env _ res _el tel ->
            match tel with
            | [(_, (shape_ty, _, _))] ->
              Typing_shapes.to_dict env p shape_ty res
            | _ -> (env, res))
      | _ -> dispatch_class_const env class_id method_id
    end
    (* Special function `parent::__construct` *)
    | Class_const ((_, pos, CIparent), ((_, construct) as id))
      when String.equal construct SN.Members.__construct ->
      let ( env,
            tel,
            typed_unpack_element,
            ty,
            pty,
            ctor_fty,
            should_forget_fakes ) =
        call_parent_construct p pos env el unpacked_element
      in
      let (env, te) =
        TUtils.make_simplify_typed_expr
          env
          fpos
          ctor_fty
          (Aast.Class_const ((pty, pos, Aast.CIparent), id))
      in
      let result =
        make_call
          env
          te
          [] (* tal: no type arguments to constructor *)
          tel
          typed_unpack_element
          ty
      in
      (result, should_forget_fakes)
    (* Calling parent / class method *)
    | Class_const (class_id, m) -> dispatch_class_const env class_id m
    (* Readonly Expressions do not affect the type, but need to be threaded through when they're part of a call *)
    | ReadonlyExpr r ->
      let env = Env.set_readonly env true in
      (* Recurse onto the inner call *)
      let ((env, expr, ty), s) =
        dispatch_call ~expected ~ctxt p env r explicit_targs el unpacked_element
      in
      (match expr with
      | (ty, _, Call { func; targs; args; unpacked_arg }) ->
        let (caller_ty, caller_pos, _) = func in
        (* Rewrap the caller in the readonly expression after we're done *)
        let (env, wrapped_caller) =
          TUtils.make_simplify_typed_expr
            env
            caller_pos
            caller_ty
            (Aast.ReadonlyExpr func)
        in
        let result = make_call env wrapped_caller targs args unpacked_arg ty in
        (result, s)
      | _ -> ((env, expr, ty), s))
    (* Call instance method *)
    | Obj_get (e1, (_, pos_id, Id m), nullflavor, Is_method)
      when not (TCO.method_call_inference (Env.get_tcopt env)) ->
      let (env, te1, ty1) =
        expr
          ~expected:None
          ~config:Config.{ default with accept_using_var = true }
          ~ctxt:Context.default
          env
          e1
      in
      let nullsafe =
        match nullflavor with
        | OG_nullthrows -> None
        | OG_nullsafe -> Some p
      in
      let (_, p1, _) = e1 in
      let ( (env, ty_err_opt),
            (tfty, tal),
            lval_ty_mismatch_opt,
            _rval_ty_mismatch_opt ) =
        TOG.obj_get_with_mismatches
          ~obj_pos:p1
          ~is_method:true
          ~inst_meth:false
          ~meth_caller:false
          ~nullsafe:(Option.map ~f:(fun p -> Reason.Rnullsafe_op p) nullsafe)
          ~coerce_from_ty:None
          ~explicit_targs
          ~class_id:(CIexpr e1)
          ~member_id:m
          ~on_error:Typing_error.Callback.unify_error
          env
          ty1
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      check_disposable_in_return env tfty;
      let (env, (tel, typed_unpack_element, ty, should_forget_fakes)) =
        call
          ~nullsafe
          ~expected
          ~expr_pos:p
          ~recv_pos:pos_id
          ~in_await:None
          env
          tfty
          el
          unpacked_element
      in
      let (env, inner_te) =
        TUtils.make_simplify_typed_expr env pos_id tfty (Aast.Id m)
      in
      let (env, te) =
        TUtils.make_simplify_typed_expr
          env
          fpos
          tfty
          (Aast.Obj_get
             ( hole_on_ty_mismatch ~ty_mismatch_opt:lval_ty_mismatch_opt te1,
               inner_te,
               nullflavor,
               Is_method ))
      in
      let result = make_call env te tal tel typed_unpack_element ty in
      (result, should_forget_fakes)
    (* Call instance method using new method call inference *)
    | Obj_get (receiver, (_, pos_id, Id meth), nullflavor, Is_method) ->
      (*****
          Typecheck `Obj_get` by enforcing that:
          - `<instance_type>` <: `Thas_member(m, #1)`
          where #1 is a fresh type variable.
        *****)
      let (env, typed_receiver, receiver_ty) =
        expr
          ~expected:None
          ~config:Config.{ default with accept_using_var = true }
          ~ctxt:Context.default
          env
          receiver
      in
      let env = might_throw ~join_pos:p env in
      let nullsafe =
        match nullflavor with
        | OG_nullthrows -> None
        | OG_nullsafe -> Some p
      in
      (* Generate a fresh type `method_ty` for the type of the
         instance method, i.e. #1 *)
      let (env, method_ty) = Env.fresh_type env p in
      (* Create `Thas_member` constraint type *)
      let (_, receiver_p, _) = receiver in
      let reason = Reason.Rwitness receiver_p in
      let has_method_ty =
        MakeType.has_member
          reason
          ~name:meth
          ~ty:method_ty
          ~class_id:(CIexpr receiver)
          ~explicit_targs:(Some explicit_targs)
      in
      let env = Env.set_tyvar_variance env method_ty in
      let (env, has_method_super_ty) =
        if Option.is_none nullsafe then
          (env, has_method_ty)
        else
          (* If null-safe, then `receiver_ty` <: `?Thas_member(m, #1)`,
             but *unlike* property access typing in `expr_`, we still use `#1` as
             our "result" if `receiver_ty` is nullable  (as opposed to `?#1`),
             deferring null-safety handling to after `call` *)
          let r = Reason.Rnullsafe_op p in
          let null_ty = MakeType.null r in
          Union.union_i env r has_method_ty null_ty
      in
      let (_, receiver_pos, _) = receiver in
      let (env, ty_err_opt) =
        Type.sub_type_i
          receiver_pos
          Reason.URnone
          env
          (LoclType receiver_ty)
          has_method_super_ty
          Typing_error.Callback.unify_error
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      let ty_nothing = MakeType.nothing Reason.none in
      let ty_mismatch_opt =
        mk_ty_mismatch_opt receiver_ty ty_nothing ty_err_opt
      in

      (* Perhaps solve for `method_ty`. Opening and closing a scope is too coarse
         here - type parameters are localised to fresh type variables over the
         course of subtyping above, and we do not want to solve these until later.
         Once we typecheck all function calls with a subtyping of function types,
         we should not need to solve early at all - transitive closure of
         subtyping should give enough information. *)
      let (env, ty_err_opt1) =
        match get_var method_ty with
        | Some var ->
          Typing_solver.solve_to_equal_bound_or_wrt_variance
            env
            Reason.Rnone
            var
        | None -> (env, None)
      in
      let localize_targ env (_, targ) = Phase.localize_targ env targ in
      let ((env, ty_err_opt), typed_targs) =
        List.map_env_ty_err_opt
          env
          ~f:(localize_targ ~check_well_kinded:true)
          explicit_targs
          ~combine_ty_errs:Typing_error.multiple_opt
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      check_disposable_in_return env method_ty;
      let ( env,
            (typed_params, typed_unpack_element, ret_ty, should_forget_fakes) )
          =
        call
          ~nullsafe
          ~expected
          ~in_await:ctxt.Context.in_await
          ~expr_pos:p
          ~recv_pos:fpos
          env
          method_ty
          el
          unpacked_element
      in
      (* If the call is nullsafe AND the receiver is nullable,
         make the return type nullable too *)
      let (env, ret_ty) =
        if Option.is_some nullsafe then
          let r = Reason.Rnullsafe_op p in
          let null_ty = MakeType.null r in
          let (env, null_or_nothing_ty) =
            Inter.intersect env ~r null_ty receiver_ty
          in
          let (env, ret_option_ty) =
            Union.union env null_or_nothing_ty ret_ty
          in
          (env, ret_option_ty)
        else
          (env, ret_ty)
      in
      let (env, inner_te1) =
        TUtils.make_simplify_typed_expr env pos_id method_ty (Aast.Id meth)
      in
      let (env, inner_te2) =
        TUtils.make_simplify_typed_expr
          env
          fpos
          method_ty
          (Aast.Obj_get
             ( hole_on_ty_mismatch ~ty_mismatch_opt typed_receiver,
               inner_te1,
               nullflavor,
               Is_method ))
      in
      let result =
        make_call
          env
          inner_te2
          typed_targs
          typed_params
          typed_unpack_element
          ret_ty
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
      (result, should_forget_fakes)
    (* Function invocation *)
    | Id id -> dispatch_id env id
    | _ ->
      let (env, te, fty) =
        expr ~expected:None ~config:Config.default ~ctxt:Context.default env e
      in
      let ((env, ty_err_opt1), fty) =
        Typing_solver.expand_type_and_solve
          ~description_of_expected:"a function value"
          env
          fpos
          fty
      in
      check_disposable_in_return env fty;
      let (env, (tel, typed_unpack_element, ty, should_forget_fakes)) =
        call
          ~expected
          ~expr_pos:p
          ~recv_pos:fpos
          ~in_await:None
          env
          fty
          el
          unpacked_element
      in
      let result =
        make_call
          env
          te
          (* tal: no type arguments to function values, as they are non-generic *)
          []
          tel
          typed_unpack_element
          ty
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
      (result, should_forget_fakes)

  and call_construct
      p env class_ params el unpacked_element (_, cid_pos, cid_) cid_ty =
    let r = Reason.Rwitness p in
    let cid_ty =
      if Nast.equal_class_id_ cid_ CIparent then
        MakeType.this r
      else
        cid_ty
    in
    let ety_env =
      {
        empty_expand_env with
        this_ty = cid_ty;
        substs = TUtils.make_locl_subst_for_class_tparams class_ params;
        on_error = Some (Typing_error.Reasons_callback.unify_error_at p);
      }
    in
    let (env, ty_err_opt) =
      Phase.check_where_constraints
        ~in_class:true
        ~use_pos:p
        ~definition_pos:(Cls.pos class_)
        ~ety_env
        env
        (Cls.where_constraints class_)
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    let cstr = Env.get_construct env class_ in
    match fst cstr with
    | None ->
      if (not (List.is_empty el)) || Option.is_some unpacked_element then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.Constructor_no_args p);
      let (env, tel, _tyl) =
        argument_list_exprs
          (expr ~expected:None ~config:Config.default ~ctxt:Context.default)
          env
          el
      in
      let should_forget_fakes = true in
      (* Construct a default __construct type *)
      let ft =
        {
          ft_tparams = [];
          ft_where_constraints = [];
          ft_params = [];
          ft_implicit_params =
            { capability = CapDefaults (Pos_or_decl.of_raw_pos p) };
          ft_flags =
            Typing_defs_flags.Fun.make
              Ast_defs.FSync
              ~return_disposable:false
              ~returns_readonly:false
              ~readonly_this:false
              ~support_dynamic_type:false
              ~is_memoized:false
              ~variadic:false;
          ft_ret = MakeType.unenforced (MakeType.void r);
          ft_cross_package = None;
        }
      in
      let ty = mk (r, Tfun ft) in
      (env, tel, None, ty, should_forget_fakes)
    | Some
        { ce_visibility = vis; ce_type = (lazy m); ce_deprecated; ce_flags; _ }
      ->
      let def_pos = get_pos m in
      Option.iter
        ~f:(Typing_error_utils.add_typing_error ~env)
        (TVis.check_obj_access ~is_method:true ~use_pos:p ~def_pos env vis);
      Option.iter
        ~f:(Typing_error_utils.add_typing_error ~env)
        (TVis.check_deprecated ~use_pos:p ~def_pos env ce_deprecated);
      (* Obtain the type of the constructor *)
      let (env, m) =
        let _r = get_reason m |> Reason.localize in
        match get_node m with
        | Tfun ft ->
          let ft =
            Typing_enforceability.compute_enforced_and_pessimize_fun_type
              ~this_class:(Some class_)
              env
              ft
          in
          Option.iter
            ~f:(Typing_error_utils.add_typing_error ~env)
            (TVis.check_cross_package
               ~use_pos:p
               ~def_pos
               env
               ft.ft_cross_package);

          (* This creates type variables for non-denotable type parameters on constructors.
           * These are notably different from the tparams on the class, which are handled
           * at the top of this function. User-written type parameters on constructors
           * are still a parse error. This is a no-op if ft.ft_tparams is empty. *)
          let ((env, ty_err_opt1), implicit_constructor_targs) =
            Phase.localize_targs
              ~check_well_kinded:true
              ~is_method:true
              ~def_pos
              ~use_pos:p
              ~use_name:"constructor"
              env
              ft.ft_tparams
              []
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
          let ((env, ty_err_opt2), ft) =
            Phase.(
              localize_ft
                ~instantiation:
                  {
                    use_name = "constructor";
                    use_pos = p;
                    explicit_targs = implicit_constructor_targs;
                  }
                ~ety_env
                ~def_pos
                env
                ft)
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
          let should_wrap =
            TCO.enable_sound_dynamic (Env.get_tcopt env)
            && (Typing_defs_flags.ClassElt.supports_dynamic_type ce_flags
               || Cls.get_support_dynamic_type class_)
          in
          ( env,
            Typing_dynamic.maybe_wrap_with_supportdyn
              ~should_wrap
              (Reason.localize (get_reason m))
              ft )
        | _ ->
          Errors.internal_error p "Expected function type for constructor";
          Env.fresh_type_error env p
      in
      let (env, (tel, typed_unpack_element, _ty, should_forget_fakes)) =
        call
          ~expected:None
          ~expr_pos:p
          ~recv_pos:cid_pos
          ~in_await:None
          env
          m
          el
          unpacked_element
      in
      (env, tel, typed_unpack_element, m, should_forget_fakes)

  and inout_write_back env { fp_type; _ } (pk, ((_, pos, _) as e)) =
    match pk with
    | Ast_defs.Pinout _ ->
      (* Translate the write-back semantics of inout parameters.
       *
       * This matters because we want to:
       * (1) make sure we can write to the original argument
       *     (modifiable lvalue check)
       * (2) allow for growing of locals / Tunions (type side effect)
       *     but otherwise unify the argument type with the parameter hint
       *)
      let (env, _te, _ty) =
        Assign.assign_ pos Reason.URparam_inout env e pos fp_type.et_type
      in
      env
    | _ -> env

  and call
      ~(expected : ExpectedTy.t option)
      ?(nullsafe : Pos.t option = None)
      ~(in_await : locl_phase Reason.t_ option)
      ?(dynamic_func : dyn_func_kind option)
      ~(expr_pos : Pos.t)
      ~(recv_pos : Pos.t)
      env
      fty
      (el : (Ast_defs.param_kind * Nast.expr) list)
      (unpacked_element : Nast.expr option) :
      env
      * ((Ast_defs.param_kind * Tast.expr) list
        * Tast.expr option
        * locl_ty
        * bool) =
    Typing_log.(
      log_with_level env "typing" ~level:1 (fun () ->
          log_types
            (Pos_or_decl.of_raw_pos expr_pos)
            env
            [
              Log_head
                ( ("Typing.call "
                  ^
                  match dynamic_func with
                  | None -> "None"
                  | Some Supportdyn_function -> "sd"
                  | Some Like_function -> "~"),
                  [Log_type ("fty", fty)] );
            ]));

    let rec is_fun_type ty =
      match get_node ty with
      | Tfun _ -> true
      | Tnewtype (name, [ty], _) when String.equal name SN.Classes.cSupportDyn
        ->
        is_fun_type ty
      | _ -> false
    in
    (* When we enter a like function, it is safe to ignore the like type if we are
       already in a supportdyn function.
       Semantically, dyn & (dyn | (t1 -> t2)) = dyn | (dyn & (t1 -> t2) = dyn, and
       doing this lets us keep like-stripping the arguments *)
    let to_like_function dynamic_func =
      match dynamic_func with
      | Some _ -> dynamic_func
      | None -> Some Like_function
    in
    let (env, tyl) =
      TUtils.get_concrete_supertypes
        ~expand_supportdyn:false
        ~abstract_enum:true
        env
        fty
    in
    if List.is_empty tyl then begin
      bad_call env expr_pos fty;
      let (env, ty_err_opt) =
        call_untyped_unpack env (get_pos fty) unpacked_element
      in
      let should_forget_fakes = true in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      let (env, ty) = Env.fresh_type_error env expr_pos in
      (env, ([], None, ty, should_forget_fakes))
    end else
      let (env, fty) = Inter.intersect_list env (get_reason fty) tyl in
      let ((env, ty_err_opt), efty) =
        if TCO.method_call_inference (Env.get_tcopt env) then
          let (env, ty) = Env.expand_type env fty in
          ((env, None), ty)
        else
          Typing_solver.expand_type_and_solve
            ~description_of_expected:"a function value"
            env
            expr_pos
            fty
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      match TUtils.try_strip_dynamic env efty with
      | Some ty
        when TCO.enable_sound_dynamic (Env.get_tcopt env) && is_fun_type ty ->
        call
          ~expected
          ~nullsafe
          ~in_await
          ?dynamic_func:(to_like_function dynamic_func)
          ~expr_pos
          ~recv_pos
          env
          ty
          el
          unpacked_element
      | _ ->
        (match deref efty with
        | (r, Tdynamic) when TCO.enable_sound_dynamic (Env.get_tcopt env) ->
          let ty = MakeType.dynamic (Reason.Rdynamic_call expr_pos) in
          let el =
            (* Need to check that the type of the unpacked_element can be,
             * coerced to dynamic, just like all of the other arguments, in addition
             * to the check below in call_untyped_unpack, that it is unpackable.
             * We don't need to unpack and check each type because a tuple is
             * coercible iff it's constituent types are. *)
            Option.value_map
              ~f:(fun u -> el @ [(Ast_defs.Pnormal, u)])
              ~default:el
              unpacked_element
          in
          let expected_arg_ty =
            ExpectedTy.make
              ~coerce:(Some Typing_logic.CoerceToDynamic)
              expr_pos
              Reason.URparam
              ty
          in
          let ((env, e1), tel) =
            List.map_env_ty_err_opt
              env
              el
              ~combine_ty_errs:Typing_error.multiple_opt
              ~f:(fun env (pk, elt) ->
                let (env, te, e_ty) =
                  match elt with
                  (* Special case for unqualified enum class label: treat as dynamic *)
                  | (_, p, EnumClassLabel (None, s)) ->
                    make_result
                      env
                      p
                      (Aast.EnumClassLabel (None, s))
                      (MakeType.dynamic (Reason.Rwitness p))
                  | _ ->
                    expr
                      ~expected:(Some expected_arg_ty)
                      ~config:Config.default
                      ~ctxt:Context.default
                      env
                      elt
                in
                let env =
                  match pk with
                  | Ast_defs.Pinout _ ->
                    let (_, pos, _) = elt in
                    let (env, _te, _ty) =
                      Assign.assign_ pos Reason.URparam_inout env elt pos efty
                    in
                    env
                  | Ast_defs.Pnormal -> env
                in
                let (env, ty_err_opt) =
                  SubType.sub_type
                    ~coerce:(Some Typing_logic.CoerceToDynamic)
                    env
                    e_ty
                    ty
                  @@ Some
                       (Typing_error.Reasons_callback.unify_error_at expr_pos)
                in
                let ty_mismatch_opt = mk_ty_mismatch_opt e_ty ty ty_err_opt in
                ( (env, ty_err_opt),
                  (pk, hole_on_ty_mismatch ~ty_mismatch_opt te) ))
          in
          let (env, e2) =
            call_untyped_unpack env (Reason.to_pos r) unpacked_element
          in
          let should_forget_fakes = true in
          let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          (env, (tel, None, ty, should_forget_fakes))
        | (r, ((Tprim Tnull | Tdynamic | Tany _ | Tunion [] | Tvar _) as ty))
          when match ty with
               | Tprim Tnull -> Option.is_some nullsafe
               | Tvar _ -> TUtils.is_tyvar_error env efty
               | _ -> true ->
          let el =
            Option.value_map
              ~f:(fun u -> el @ [(Ast_defs.Pnormal, u)])
              ~default:el
              unpacked_element
          in
          let (env, tel) =
            List.map_env env el ~f:(fun env (pk, elt) ->
                let (env, te, _ty) =
                  expr
                    ~expected:None
                    ~config:Config.default
                    ~ctxt:Context.default
                    env
                    elt
                in
                let env =
                  match pk with
                  | Ast_defs.Pinout _ ->
                    let (_, pos, _) = elt in
                    let (env, _te, _ty) =
                      Assign.assign_ pos Reason.URparam_inout env elt pos efty
                    in
                    env
                  | Ast_defs.Pnormal -> env
                in
                (env, (pk, te)))
          in
          let (env, ty_err_opt) =
            call_untyped_unpack env (Reason.to_pos r) unpacked_element
          in
          let (env, ty) =
            match ty with
            | Tprim Tnull -> (env, mk (r, Tprim Tnull))
            | Tdynamic -> (env, MakeType.dynamic (Reason.Rdynamic_call expr_pos))
            | Tvar _ when TUtils.is_tyvar_error env efty ->
              Env.fresh_type_error env expr_pos
            | Tunion []
            | _ (* _ should not happen! *) ->
              (env, mk (r, Tunion []))
          in
          let should_forget_fakes = true in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          (env, (tel, None, ty, should_forget_fakes))
        | (_, Tunion [ty]) ->
          call
            ~expected
            ~nullsafe
            ~in_await
            ?dynamic_func
            ~expr_pos
            ~recv_pos
            env
            ty
            el
            unpacked_element
        | (r, Tunion tyl) ->
          let (env, resl) =
            List.map_env env tyl ~f:(fun env ty ->
                call
                  ~expected
                  ~nullsafe
                  ~in_await
                  ?dynamic_func
                  ~expr_pos
                  ~recv_pos
                  env
                  ty
                  el
                  unpacked_element)
          in
          let should_forget_fakes =
            List.exists resl ~f:(fun (_, _, _, forget) -> forget)
          in
          let retl = List.map resl ~f:(fun (_, _, x, _) -> x) in
          let (env, ty) = Union.union_list env r retl in
          (* We shouldn't be picking arbitrarily for the TAST here, as TAST checks
           * depend on the types inferred. Here's we're preserving legacy behaviour
           * by picking the last one.
           * TODO: don't do this, instead use subtyping to push unions
           * through function types
           *)
          let (tel, typed_unpack_element, _, _) = List.hd_exn (List.rev resl) in
          (env, (tel, typed_unpack_element, ty, should_forget_fakes))
        | (r, Tintersection tyl) ->
          let (env, resl) =
            TUtils.run_on_intersection env tyl ~f:(fun env ty ->
                call
                  ~expected
                  ~nullsafe
                  ~in_await
                  ?dynamic_func
                  ~expr_pos
                  ~recv_pos
                  env
                  ty
                  el
                  unpacked_element)
          in
          let should_forget_fakes =
            List.for_all resl ~f:(fun (_, _, _, forget) -> forget)
          in
          let retl = List.map resl ~f:(fun (_, _, x, _) -> x) in
          let (env, ty) = Inter.intersect_list env r retl in
          (* We shouldn't be picking arbitrarily for the TAST here, as TAST checks
           * depend on the types inferred. Here we're preserving legacy behaviour
           * by picking the last one.
           * TODO: don't do this, instead use subtyping to push intersections
           * through function types
           *)
          let (tel, typed_unpack_element, _, _) = List.hd_exn (List.rev resl) in
          (env, (tel, typed_unpack_element, ty, should_forget_fakes))
        | (r2, Tfun ft) ->
          (* Typing of format string functions. It is dependent on the arguments (el)
           * so it cannot be done earlier.
           *)
          let pos_def = Reason.to_pos r2 in
          let (env, ft) = Typing_exts.retype_magic_func env ft el in
          let (env, var_param) = variadic_param env ft in
          (* Force subtype with expected result *)
          let env =
            check_expected_ty "Call result" env ft.ft_ret.et_type expected
          in
          let env = Env.set_tyvar_variance env ft.ft_ret.et_type in
          let is_lambda (_, _, e) =
            match e with
            | Efun _
            | Lfun _ ->
              true
            | _ -> false
          in
          let get_next_param_info paraml =
            match paraml with
            | param :: paraml -> (false, Some param, paraml)
            | [] -> (true, var_param, paraml)
          in
          let rec compute_enum_name env lty =
            match get_node lty with
            | Tclass ((_, enum_name), _, _) when Env.is_enum_class env enum_name
              ->
              (env, Some enum_name)
            | Tgeneric (name, _) ->
              let (env, upper_bounds) =
                TUtils.collect_enum_class_upper_bounds env name
              in
              begin
                match upper_bounds with
                | None -> (env, None)
                | Some upper_bound -> begin
                  (* To avoid ambiguity, we only support the case where
                   * there is a single upper bound that is an EnumClass.
                   * We might want to relax that later (e.g. with  the
                   * support for intersections).
                   *)
                  match get_node upper_bound with
                  | Tclass ((_, name), _, _) when Env.is_enum_class env name ->
                    (env, Some name)
                  | _ -> (env, None)
                end
              end
            | Tvar var ->
              (* minimal support to only deal with Tvar when:
               * - it is the valueOf from BuiltinEnumClass.
               *   In this case, we know the tvar as a single lowerbound,
               *   `this` which must be an enum class.
               * - it is a generic "TX::T" from a where clause. We try
               *   to look at an upper bound, if it is an enum class, or fail.
               *
               * We could relax this in the future but
               * I want to avoid complex constraints for now.
               *)
              let lower_bounds = Env.get_tyvar_lower_bounds env var in
              if ITySet.cardinal lower_bounds <> 1 then
                (env, None)
              else (
                match ITySet.choose lower_bounds with
                | ConstraintType _ -> (env, None)
                | LoclType lower ->
                  (match get_node lower with
                  | Tclass ((_, enum_name), _, _)
                    when Env.is_enum_class env enum_name ->
                    (env, Some enum_name)
                  | Tgeneric _ -> compute_enum_name env lower
                  | _ -> (env, None))
              )
            | _ ->
              (* Already reported, see Typing_type_wellformedness *)
              (env, None)
          in
          let check_arg
              env param_kind ((_, pos, arg) as e) opt_param ~is_variadic =
            match opt_param with
            | Some param ->
              (* First check if the parameter is a HH\EnumClass\Label.  *)
              let (env, label_type) =
                let ety = param.fp_type.et_type in
                let (env, ety) = Env.expand_type env ety in
                let is_label env ety =
                  match get_node (TUtils.strip_dynamic env ety) with
                  | Tnewtype (name, [ty_enum; _ty_interface], _) ->
                    if String.equal SN.Classes.cEnumClassLabel name then
                      Some (name, ty_enum)
                    else
                      None
                  | _ -> None
                in
                let is_maybe_label =
                  match get_node (TUtils.strip_dynamic env ety) with
                  | Toption ety -> is_label env ety
                  | _ -> is_label env ety
                in
                match (arg, is_maybe_label) with
                | (Aast.EnumClassLabel (None, label_name), Some (name, ty_enum))
                  ->
                  let ctor = name in
                  (match compute_enum_name env ty_enum with
                  | (env, None) -> (env, EnumClassLabelOps.ClassNotFound)
                  | (env, Some enum_name) ->
                    let ty_pos = Typing_defs.get_pos ty_enum in
                    EnumClassLabelOps.expand
                      pos
                      env
                      ~full:false
                      ~ctor
                      (Pos.none, enum_name)
                      label_name
                      (Some ty_pos))
                | (Aast.EnumClassLabel (Some _, _), _) ->
                  (* Full info is here, use normal inference *)
                  (env, EnumClassLabelOps.Skip)
                | (_, _) -> (env, EnumClassLabelOps.Skip)
              in
              let (env, te, ty) =
                match label_type with
                | EnumClassLabelOps.Success (te, ty)
                | EnumClassLabelOps.LabelNotFound (te, ty) ->
                  (env, te, ty)
                | _ ->
                  (* Expected type has a like type for checking arguments
                   * to supportdyn functions
                   *)
                  let (env, pess_type) =
                    match dynamic_func with
                    | Some Supportdyn_function ->
                      Typing_array_access.pessimise_type
                        env
                        param.fp_type.et_type
                    | _ -> (env, param.fp_type.et_type)
                  in
                  let expected =
                    ExpectedTy.make_and_allow_coercion_opt
                      env
                      pos
                      Reason.URparam
                      {
                        et_type = pess_type;
                        et_enforced = param.fp_type.et_enforced;
                      }
                  in
                  expr
                    ~expected
                    ~config:
                      Config.
                        {
                          default with
                          accept_using_var = get_fp_accept_disposable param;
                        }
                    ~ctxt:Context.default
                    env
                    e
              in
              let (env, ty_mismatch_opt, used_dynamic) =
                check_argument_type_against_parameter_type
                  ~dynamic_func
                  env
                  param
                  param_kind
                  (e, ty)
                  ~is_variadic
              in
              ( env,
                Some (hole_on_ty_mismatch ~ty_mismatch_opt te, ty),
                used_dynamic )
            | None ->
              let (env, te, ty) =
                expr
                  ~expected:None
                  ~config:Config.default
                  ~ctxt:Context.default
                  env
                  e
              in
              (env, Some (te, ty), false)
          in
          let set_tyvar_variance_from_lambda_param env opt_param =
            match opt_param with
            | Some param ->
              let rec set_params_variance env ty =
                let ty = TUtils.strip_dynamic env ty in
                let (env, ty) = Env.expand_type env ty in
                match get_node ty with
                | Tunion [ty] -> set_params_variance env ty
                | Toption ty -> set_params_variance env ty
                | Tfun { ft_params; ft_ret; _ } ->
                  let env =
                    List.fold
                      ~init:env
                      ~f:(fun env param ->
                        Env.set_tyvar_variance env param.fp_type.et_type)
                      ft_params
                  in
                  Env.set_tyvar_variance env ft_ret.et_type ~flip:true
                | Tnewtype (name, [ty], _)
                  when String.equal name SN.Classes.cSupportDyn ->
                  set_params_variance env ty
                | _ -> env
              in
              set_params_variance env param.fp_type.et_type
            | None -> env
          in
          (* Given an expected function type ft, check types for the non-unpacked
           * arguments. Don't check lambda expressions if check_lambdas=false *)
          let rec check_args_aux check_lambdas env el paraml used_dynamic rl =
            match el with
            (* We've got an argument *)
            | ((pk, e), opt_result) :: el ->
              (* Pick up next parameter type info *)
              let (is_variadic, opt_param, paraml) =
                get_next_param_info paraml
              in
              let (env, one_result, used_dynamic') =
                match (check_lambdas, is_lambda e) with
                | (false, false)
                | (true, true) ->
                  check_arg env pk e opt_param ~is_variadic
                | (false, true) ->
                  let env =
                    set_tyvar_variance_from_lambda_param env opt_param
                  in
                  (env, opt_result, false)
                | (true, false) -> (env, opt_result, false)
              in
              check_args_aux
                check_lambdas
                env
                el
                paraml
                (used_dynamic || used_dynamic')
                (((pk, e), one_result) :: rl)
            | [] -> (env, rl, paraml, used_dynamic)
          in
          let check_args check_lambdas env el paraml =
            let (env, rl, paraml, used_dynamic) =
              check_args_aux check_lambdas env el paraml false []
            in
            (env, List.rev rl, paraml, used_dynamic)
          in
          (* Same as above, but checks the types of the implicit arguments, which are
           * read from the context *)
          let check_implicit_args env =
            let capability =
              Typing_coeffects.get_type ft.ft_implicit_params.capability
            in
            let should_skip_check =
              (not (TCO.call_coeffects (Env.get_tcopt env)))
              (* When inside an expression tree, expressions are virtualized and
                 thus never executed. Safe to skip coeffect checks in this case. *)
              || Env.is_in_expr_tree env
            in
            if should_skip_check then
              (env, None)
            else
              let env_capability =
                (Env.get_local_check_defined
                   env
                   (expr_pos, Typing_coeffects.capability_id))
                  .Typing_local_types.ty
              in
              let base_error =
                Typing_error.Primary.(
                  Coeffect
                    (Coeffect.Call_coeffect
                       {
                         pos = recv_pos;
                         available_incl_unsafe =
                           Typing_coeffects.pretty env env_capability;
                         available_pos = Typing_defs.get_pos env_capability;
                         required_pos = Typing_defs.get_pos capability;
                         required = Typing_coeffects.pretty env capability;
                       }))
              in
              Type.sub_type expr_pos Reason.URnone env env_capability capability
              @@ Typing_error.Callback.always base_error
          in
          let should_forget_fakes =
            (* If the function doesn't have write priveleges to properties, fake
               members cannot be reassigned, so their refinements stand. *)
            let capability =
              Typing_coeffects.get_type ft.ft_implicit_params.capability
            in
            SubType.is_sub_type
              env
              capability
              (MakeType.capability Reason.Rnone SN.Capabilities.writeProperty)
          in

          (* First check the non-lambda arguments. For generic functions, this
           * is likely to resolve type variables to concrete types *)
          let rl = List.map el ~f:(fun e -> (e, None)) in
          let non_variadic_ft_params =
            if get_ft_variadic ft then
              List.drop_last_exn ft.ft_params
            else
              ft.ft_params
          in
          let (env, rl, _, used_dynamic1) =
            check_args false env rl non_variadic_ft_params
          in
          (* Now check the lambda arguments, hopefully with type variables resolved *)
          let (env, rl, paraml, used_dynamic2) =
            check_args true env rl non_variadic_ft_params
          in
          (* We expect to see results for all arguments after this second pass *)
          let get_param ((pk, _), opt) =
            match opt with
            | Some (((_, pos, _) as e), ty) -> ((pk, e), (pos, ty))
            | None -> failwith "missing parameter in check_args"
          in
          let (tel, argtys) =
            let l = List.map rl ~f:get_param in
            List.unzip l
          in
          let (env, ty_err_opt) = check_implicit_args env in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          let (env, typed_unpack_element, arity, did_unpack, used_dynamic3) =
            match unpacked_element with
            | None -> (env, None, List.length el, false, false)
            | Some e ->
              (* Now that we're considering an splat (Some e) we need to construct a type that
               * represents the remainder of the function's parameters. `paraml` represents those
               * remaining parameters, and the variadic parameter is stored in `var_param`. For example, given
               *
               * function f(int $i, string $j, float $k = 3.14, mixed ...$m): void {}
               * function g((string, float, bool) $t): void {
               *   f(3, ...$t);
               * }
               *
               * the constraint type we want is splat([#1], [opt#2], #3).
               *)
              let (consumed, required_params, optional_params) =
                split_remaining_params_required_optional ft paraml
              in
              let (_, p1, _) = e in
              let (env, (d_required, d_optional, d_variadic)) =
                generate_splat_type_vars
                  env
                  p1
                  required_params
                  optional_params
                  var_param
              in
              let destructure_ty =
                ConstraintType
                  (mk_constraint_type
                     ( Reason.Runpack_param (p1, pos_def, consumed),
                       Tdestructure
                         {
                           d_required;
                           d_optional;
                           d_variadic;
                           d_kind = SplatUnpack;
                         } ))
              in
              let (env, te, ty) =
                expr
                  ~expected:None
                  ~config:Config.default
                  ~ctxt:Context.default
                  env
                  e
              in
              (* Populate the type variables from the expression in the splat *)
              let (env, ty_err_opt) =
                Type.sub_type_i
                  p1
                  Reason.URparam
                  env
                  (LoclType ty)
                  destructure_ty
                  Typing_error.Callback.unify_error
              in
              let (env, te, used_dynamic) =
                match ty_err_opt with
                | Some _ ->
                  (* Our type cannot be destructured, add a hole with `nothing`
                     as expected type *)
                  let ty_expect =
                    MakeType.nothing
                    @@ Reason.Rsolve_fail (Pos_or_decl.of_raw_pos expr_pos)
                  in
                  (env, mk_hole te ~ty_have:ty ~ty_expect, false)
                | None ->
                  (* We have a type that can be destructured so continue and use
                     the type variables for the remaining parameters *)
                  let (env, err_opts, used_dynamic) =
                    List.fold2_exn
                      ~init:(env, [], false)
                      d_required
                      required_params
                      ~f:(fun (env, errs, used_dynamic_acc) elt param ->
                        let (env, err_opt, used_dynamic) =
                          check_argument_type_against_parameter_type
                            ~dynamic_func
                            env
                            param
                            Ast_defs.Pnormal
                            (e, elt)
                            ~is_variadic:false
                        in
                        (env, err_opt :: errs, used_dynamic_acc || used_dynamic))
                  in
                  let (env, err_opts, used_dynamic) =
                    List.fold2_exn
                      ~init:(env, err_opts, used_dynamic)
                      d_optional
                      optional_params
                      ~f:(fun (env, errs, used_dynamic_acc) elt param ->
                        let (env, err_opt, used_dynamic) =
                          check_argument_type_against_parameter_type
                            ~dynamic_func
                            env
                            param
                            Ast_defs.Pnormal
                            (e, elt)
                            ~is_variadic:false
                        in
                        (env, err_opt :: errs, used_dynamic_acc || used_dynamic))
                  in
                  let (env, var_err_opt, var_used_dynamic) =
                    Option.map2 d_variadic var_param ~f:(fun v vp ->
                        check_argument_type_against_parameter_type
                          ~dynamic_func
                          env
                          vp
                          Ast_defs.Pnormal
                          (e, v)
                          ~is_variadic:true)
                    |> Option.value ~default:(env, None, false)
                  in
                  let subtyping_errs = (List.rev err_opts, var_err_opt) in
                  let te =
                    match (List.filter_map ~f:Fn.id err_opts, var_err_opt) with
                    | ([], None) -> te
                    | _ ->
                      let (_, pos, _) = te in
                      hole_on_ty_mismatch
                        te
                        ~ty_mismatch_opt:
                          (Some (ty, pack_errs pos ty subtyping_errs))
                  in
                  (env, te, used_dynamic || var_used_dynamic)
              in
              Option.iter
                ~f:(Typing_error_utils.add_typing_error ~env)
                ty_err_opt;
              ( env,
                Some te,
                List.length el + List.length d_required,
                Option.is_some d_variadic,
                used_dynamic )
          in
          let used_dynamic = used_dynamic1 || used_dynamic2 || used_dynamic3 in
          (* If dynamic_func is set, then the function type is supportdyn<t1 ... tn -> t>
             or ~(t1 ... tn -> t)
             and we are trying to call it as though it were dynamic. Hence all of the
             arguments must be subtypes of dynamic, regardless of whether they have
             a like type. *)
          let env =
            if used_dynamic then begin
              let rec check_args_dynamic env argtys =
                match argtys with
                (* We've got an argument *)
                | (pos, argty) :: argtys ->
                  let (env, ty_err_opt) =
                    TUtils.supports_dynamic env argty
                    @@ Some (Typing_error.Reasons_callback.unify_error_at pos)
                  in
                  Option.iter
                    ~f:(Typing_error_utils.add_typing_error ~env)
                    ty_err_opt;
                  check_args_dynamic env argtys
                | [] -> env
              in
              check_args_dynamic env argtys
            end else
              env
          in
          (* If we unpacked an array, we don't check arity exactly. Since each
           * unpacked array consumes 1 or many parameters, it is nonsensical to say
           * that not enough args were passed in (so we don't do the min check).
           *)
          let () = check_arity ~did_unpack env expr_pos pos_def ft arity in
          (* Variadic params cannot be inout so we can stop early *)
          let env =
            wfold_left2 inout_write_back env non_variadic_ft_params el
          in
          let ret =
            match dynamic_func with
            | Some _ when used_dynamic ->
              MakeType.locl_like r2 ft.ft_ret.et_type
            | _ -> ft.ft_ret.et_type
          in
          (env, (tel, typed_unpack_element, ret, should_forget_fakes))
        | (r, Tvar _) when TCO.method_call_inference (Env.get_tcopt env) ->
          (*
            Typecheck calls with unresolved function type by constructing a
            suitable function type from the arguments and invoking subtyping.
          *)
          let (env, typed_el, type_of_el) =
            argument_list_exprs
              (expr
                 ~expected:None
                 ~config:Config.{ default with accept_using_var = true }
                 ~ctxt:Context.default)
              env
              el
          in
          let (env, typed_unpacked_element, type_of_unpacked_element) =
            match unpacked_element with
            | Some unpacked ->
              let (env, typed_unpacked, type_of_unpacked) =
                expr
                  ~expected:None
                  ~config:Config.{ default with accept_using_var = true }
                  ~ctxt:Context.default
                  env
                  unpacked
              in
              (env, Some typed_unpacked, Some type_of_unpacked)
            | None -> (env, None, None)
          in
          let mk_function_supertype
              env pos (type_of_el, type_of_unpacked_element) =
            let mk_fun_param ty =
              let flags =
                (* Keep supertype as permissive as possible: *)
                make_fp_flags
                  ~mode:FPnormal (* TODO: deal with `inout` parameters *)
                  ~accept_disposable:false (* TODO: deal with disposables *)
                  ~has_default:false
                  ~readonly:false
              in
              {
                fp_pos = Pos_or_decl.of_raw_pos pos;
                fp_name = None;
                fp_type = MakeType.enforced ty;
                fp_flags = flags;
              }
            in
            let ft_arity =
              match type_of_unpacked_element with
              | Some type_of_unpacked ->
                let fun_param = mk_fun_param type_of_unpacked in
                [fun_param]
              | None -> []
            in
            (* TODO: ensure `ft_params`/`ft_where_constraints` don't affect subtyping *)
            let ft_tparams = [] in
            let ft_where_constraints = [] in
            let ft_cross_package = None in
            let ft_params = List.map ~f:mk_fun_param type_of_el in
            let ft_implicit_params =
              {
                capability =
                  CapDefaults (Pos_or_decl.of_raw_pos pos)
                  (* TODO(coeffects) should this be a different type? *);
              }
            in
            let (env, return_ty) = Env.fresh_type env pos in
            let return_ty =
              match in_await with
              | None -> return_ty
              | Some r -> MakeType.awaitable r return_ty
            in
            let ft_ret = MakeType.enforced return_ty in
            let ft_flags =
              (* Keep supertype as permissive as possible: *)
              Typing_defs_flags.Fun.make
                Ast_defs.FSync (* `FSync` fun can still return `Awaitable<_>` *)
                ~return_disposable:false (* TODO: deal with disposable return *)
                ~returns_readonly:false
                ~readonly_this:false
                ~support_dynamic_type:false
                ~is_memoized:false
                ~variadic:(not (List.is_empty ft_arity))
            in
            let fun_locl_type =
              {
                ft_tparams;
                ft_where_constraints;
                ft_params = ft_params @ ft_arity;
                ft_implicit_params;
                ft_ret;
                ft_flags;
                ft_cross_package;
              }
            in
            let fun_type = mk (r, Tfun fun_locl_type) in
            let env = Env.set_tyvar_variance env fun_type in
            (env, fun_type, return_ty)
          in
          let (env, fun_type, return_ty) =
            mk_function_supertype
              env
              expr_pos
              (type_of_el, type_of_unpacked_element)
          in
          let (env, ty_err_opt) =
            Type.sub_type
              expr_pos
              Reason.URnone
              env
              efty
              fun_type
              Typing_error.Callback.unify_error
          in
          let should_forget_fakes = true in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          ( env,
            (typed_el, typed_unpacked_element, return_ty, should_forget_fakes)
          )
        | (_, Tnewtype (name, [ty], _))
          when String.equal name SN.Classes.cSupportDyn ->
          let (env, ty) = Env.expand_type env ty in
          begin
            match get_node ty with
            (* If we have a function type of the form supportdyn<(function(t):~u)> then it does no
             * harm to treat it as supportdyn<(function(~t):~u)> and may produce a more precise
             * type for function application e.g. consider
             *   function expect<T as supportdyn<mixed> >(T $obj)[]: ~Invariant<T>;
             * If we have an argument of type ~t then checking against this signature will
             * produce ~t <: T <: supportdyn<mixed>, so T will be assigned a like type.
             * But if we check against the signature transformed as above, we will get
             * t <: T <: supportdyn<mixed> which is more precise.
             *)
            | Tfun ft
              when Option.is_some
                     (TUtils.try_strip_dynamic env ft.ft_ret.et_type)
                   && List.length el = 1 ->
              let ft_params =
                List.map ft.ft_params ~f:(fun fp ->
                    {
                      fp with
                      fp_type =
                        {
                          fp.fp_type with
                          et_type = TUtils.make_like env fp.fp_type.et_type;
                        };
                    })
              in
              let ty = mk (get_reason ty, Tfun { ft with ft_params }) in
              call
                ~expected
                ~nullsafe
                ~in_await
                ?dynamic_func
                ~expr_pos
                ~recv_pos
                env
                ty
                el
                unpacked_element
            | _ ->
              call
                ~expected
                ~nullsafe
                ~in_await
                ?dynamic_func:(Some Supportdyn_function)
                ~expr_pos
                ~recv_pos
                env
                ty
                el
                unpacked_element
          end
        | _ ->
          if not (TUtils.is_tyvar_error env efty) then
            bad_call env expr_pos efty;
          let (env, ty_err_opt) =
            call_untyped_unpack env (get_pos efty) unpacked_element
          in
          let should_forget_fakes = true in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          let (env, ty) = Env.fresh_type_error env expr_pos in
          (env, ([], None, ty, should_forget_fakes)))

  and call_untyped_unpack env f_pos unpacked_element =
    match unpacked_element with
    (* In the event that we don't have a known function call type, we can still
     * verify that any unpacked arguments (`...$args`) are something that can
     * be actually unpacked. *)
    | None -> (env, None)
    | Some e ->
      let (env, _, ety) =
        expr ~expected:None ~config:Config.default ~ctxt:Context.default env e
      in
      let (_, p, _) = e in
      let (env, ty) = Env.fresh_type env p in
      let destructure_ty =
        MakeType.simple_variadic_splat (Reason.Runpack_param (p, f_pos, 0)) ty
      in
      Type.sub_type_i
        p
        Reason.URnone
        env
        (LoclType ety)
        destructure_ty
        Typing_error.Callback.unify_error

  and upcast env p expr_ty hint_ty =
    let (env, ty_err_opt) =
      SubType.sub_type
        ~coerce:(Some Typing_logic.CoerceToDynamic)
        env
        expr_ty
        hint_ty
      @@ Some (Typing_error.Reasons_callback.unify_error_at p)
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    env

  and condition env tparamet ((ty, p, e) as te : Tast.expr) =
    match e with
    | Aast.Hole (e, _, _, _) -> condition env tparamet e
    | Aast.True when not tparamet ->
      ( LEnv.drop_cont env C.Next,
        { lset = Local_id.Set.empty; pkgs = SSet.empty } )
    | Aast.False when tparamet ->
      ( LEnv.drop_cont env C.Next,
        { lset = Local_id.Set.empty; pkgs = SSet.empty } )
    | Aast.Call
        {
          func = (_, _, Aast.Id (_, f));
          args = [(_, te)];
          unpacked_arg = None;
          _;
        }
      when String.equal SN.StdlibFunctions.is_null f ->
      let (env, lset) = condition_nullity ~nonnull:(not tparamet) env te in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Binop
        {
          bop = Ast_defs.Eqeq | Ast_defs.Eqeqeq;
          lhs = (_, _, Aast.Null);
          rhs = e;
        }
    | Aast.Binop
        {
          bop = Ast_defs.Eqeq | Ast_defs.Eqeqeq;
          lhs = e;
          rhs = (_, _, Aast.Null);
        } ->
      let (env, lset) = condition_nullity ~nonnull:(not tparamet) env e in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Binop
        {
          bop = Ast_defs.Eqeq | Ast_defs.Eqeqeq;
          lhs = (lhs_ty, _, _) as lhs;
          rhs = (rhs_ty, _, _) as rhs;
        }
      when tparamet ->
      let (env, lset_lhs) = refine_for_equality p env lhs rhs_ty in
      let (env, lset_rhs) = refine_for_equality p env rhs lhs_ty in
      let lset = Local_id.Set.union lset_lhs lset_rhs in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Lvar _
    | Aast.Obj_get _
    | Aast.Class_get _
    | Aast.Binop { bop = Ast_defs.Eq None; _ } ->
      let (env, ety) = Env.expand_type env ty in
      (match get_node ety with
      | Tprim Tbool -> (env, { lset = Local_id.Set.empty; pkgs = SSet.empty })
      | _ ->
        let (env, lset) = condition_nullity ~nonnull:tparamet env te in
        (env, { lset; pkgs = SSet.empty }))
    | Aast.Binop
        { bop = (Ast_defs.Diff | Ast_defs.Diff2) as op; lhs = e1; rhs = e2 } ->
      let op =
        if Ast_defs.(equal_bop op Diff) then
          Ast_defs.Eqeq
        else
          Ast_defs.Eqeqeq
      in
      condition
        env
        (not tparamet)
        (ty, p, Aast.Binop { bop = op; lhs = e1; rhs = e2 })
    (* Conjunction of conditions. Matches the two following forms:
        if (cond1 && cond2)
        if (!(cond1 || cond2))
    *)
    | Aast.Binop
        { bop = (Ast_defs.Ampamp | Ast_defs.Barbar) as bop; lhs = e1; rhs = e2 }
      when Bool.equal tparamet Ast_defs.(equal_bop bop Ampamp) ->
      let (env, { lset = lset1; pkgs = pkgs1 }) = condition env tparamet e1 in
      (* This is necessary in case there is an assignment in e2
       * We essentially redo what has been undone in the
       * `Binop (Ampamp|Barbar)` case of `expr` *)
      let (env, _, _) =
        expr
          ~expected:None
          ~config:Config.default
          ~ctxt:Context.default
          env
          (Tast.to_nast_expr e2)
      in
      let (env, { lset = lset2; pkgs = pkgs2 }) = condition env tparamet e2 in
      let lset = Local_id.Set.union lset1 lset2 in
      let pkgs =
        if tparamet then
          SSet.union pkgs1 pkgs2
        else
          SSet.empty
      in
      (env, { lset; pkgs })
    (* Disjunction of conditions. Matches the two following forms:
        if (cond1 || cond2)
        if (!(cond1 && cond2))
    *)
    | Aast.Binop
        { bop = (Ast_defs.Ampamp | Ast_defs.Barbar) as bop; lhs = e1; rhs = e2 }
      when Bool.equal tparamet Ast_defs.(equal_bop bop Barbar) ->
      let (env, { lset = lset1; _ }, { lset = lset2; _ }) =
        branch
          ~join_pos:p
          env
          (fun env ->
            (* Either cond1 is true and we don't know anything about cond2... *)
            condition env tparamet e1)
          (fun env ->
            (* ... Or cond1 is false and therefore cond2 must be true *)
            let (env, _) = condition env (not tparamet) e1 in
            (* Similarly to the conjunction case, there might be an assignment in
               cond2 which we must account for. Again we redo what has been undone in
               the `Binop (Ampamp|Barbar)` case of `expr` *)
            let (env, _, _) =
              expr
                ~expected:None
                ~config:Config.default
                ~ctxt:Context.default
                env
                (Tast.to_nast_expr e2)
            in
            condition env tparamet e2)
      in
      (env, { lset = Local_id.Set.union lset1 lset2; pkgs = SSet.empty })
    | Aast.Call
        {
          func = (_, p, Aast.Id (_, f));
          args = [(_, lv)];
          unpacked_arg = None;
          _;
        }
      when tparamet && String.equal f SN.StdlibFunctions.is_dict_or_darray ->
      let (env, lset) = safely_refine_is_array env HackDictOrDArray p f lv in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Call
        {
          func = (_, p, Aast.Id (_, f));
          args = [(_, lv)];
          unpacked_arg = None;
          _;
        }
      when tparamet && String.equal f SN.StdlibFunctions.is_vec_or_varray ->
      let (env, lset) = safely_refine_is_array env HackVecOrVArray p f lv in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Call
        {
          func = (_, p, Aast.Id (_, f));
          args = [(_, lv)];
          unpacked_arg = None;
          _;
        }
      when String.equal f SN.StdlibFunctions.is_any_array ->
      let (env, lset) =
        refine_for_is
          ~hint_first:true
          env
          tparamet
          lv
          (Reason.Rpredicated (p, f))
          (p, Happly ((p, "\\HH\\AnyArray"), [(p, Hwildcard); (p, Hwildcard)]))
      in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Call
        {
          func =
            ( _,
              _,
              Aast.Class_const
                ((_, _, Aast.CI (_, class_name)), (_, method_name)) );
          args = [(_, shape); (_, field)];
          unpacked_arg = None;
          _;
        }
      when tparamet
           && String.equal class_name SN.Shapes.cShapes
           && String.equal method_name SN.Shapes.keyExists ->
      let (env, lset) = key_exists env p shape field in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Unop (Ast_defs.Unot, e) -> condition env (not tparamet) e
    | Aast.Is (ivar, h) ->
      let (env, lset) =
        refine_for_is ~hint_first:false env tparamet ivar (Reason.Ris (fst h)) h
      in
      (env, { lset; pkgs = SSet.empty })
    | Aast.Package (_, pkg) when tparamet ->
      (env, { lset = Local_id.Set.empty; pkgs = SSet.singleton pkg })
    | _ -> (env, { lset = Local_id.Set.empty; pkgs = SSet.empty })

  and string2 env idl =
    let (env, tel) =
      List.fold_left idl ~init:(env, []) ~f:(fun (env, tel) x ->
          let (env, te, ty) =
            expr
              ~expected:None
              ~config:Config.default
              ~ctxt:Context.default
              env
              x
          in
          let (_, p, _) = x in
          if TCO.enable_strict_string_concat_interp (Env.get_tcopt env) then (
            let r = Reason.Rinterp_operand p in
            let (env, formatter_tyvar) = Env.fresh_type_invariant env p in
            let stringlike =
              MakeType.union
                r
                [
                  MakeType.arraykey r;
                  MakeType.dynamic r;
                  MakeType.hh_formatstring r formatter_tyvar;
                ]
            in
            let (env, ty_err_opt) =
              Typing_ops.sub_type
                p
                Reason.URstr_interp
                env
                ty
                stringlike
                Typing_error.Callback.strict_str_interp_type_mismatch
            in
            let ty_mismatch_opt = mk_ty_mismatch_opt ty stringlike ty_err_opt in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            (env, hole_on_ty_mismatch ~ty_mismatch_opt te :: tel)
          ) else
            let env = Typing_substring.sub_string p env ty in
            (env, te :: tel))
    in
    (env, List.rev tel)

  (* Calls the method of a class, but allows the f callback to override the
   * return value type *)
  and overload_function
      make_call
      fpos
      p
      env
      ((_, cid_pos, _) as class_id)
      method_id
      el
      unpacked_element
      f =
    let (env, _tal, tcid, ty) = Class_id.class_expr env [] class_id in
    let (env, (fty, tal)) =
      Class_get_expr.class_get
        ~is_method:true
        ~is_const:false
        ~transform_fty:None
        ~coerce_from_ty:None
        env
        ty
        method_id
        class_id
    in
    let (env, (tel, typed_unpack_element, res, should_forget_fakes)) =
      call
        ~expected:None
        ~expr_pos:p
        ~recv_pos:cid_pos
        ~in_await:None
        env
        fty
        el
        unpacked_element
    in
    let (env, ty) = f env fty res el tel in
    let (env, fty) = Env.expand_type env fty in
    let fty =
      map_ty fty ~f:(function
          | Tfun ft -> Tfun { ft with ft_ret = MakeType.unenforced ty }
          | ty -> ty)
    in
    let (env, te) =
      TUtils.make_simplify_typed_expr
        env
        fpos
        fty
        (Aast.Class_const (tcid, method_id))
    in
    (make_call env te tal tel typed_unpack_element ty, should_forget_fakes)

  and update_array_type ~lhs_of_null_coalesce p env e1 valkind =
    match valkind with
    | Valkind.Lvalue
    | Valkind.Lvalue_subexpr -> begin
      let (env, te1, ty1) =
        raw_expr
          ~expected:None
          ~config:Config.default
          ~ctxt:Context.{ default with valkind = Valkind.Lvalue_subexpr }
          env
          e1
      in
      match e1 with
      | (_, _, Lvar (_, x)) ->
        let env =
          if not (Env.is_local_present env x) then
            (* If the Lvar wasn't in the environment, add it in to avoid reporting
               subsequent errors. It has no bound since it wasn't a typed local. *)
            set_local ~is_defined:true ~bound_ty:None env (p, x) ty1
          else
            env
        in
        (env, te1, ty1)
      | _ -> (env, te1, ty1)
    end
    | Valkind.Other ->
      raw_expr
        ~expected:None
        ~config:Config.default
        ~ctxt:Context.{ default with lhs_of_null_coalesce }
        env
        e1
end

and Stmt : sig
  val stmt : env -> Nast.stmt -> env * Tast.stmt

  val block : env -> Nast.stmt list -> env * Tast.stmt list
end = struct
  let rec block env stl =
    Env.with_origin env Decl_counters.Body @@ fun env ->
    (* To insert an `AssertEnv`, `stmt` might return a `Block`. We eliminate it here
       to keep ASTs `Block`-free. *)
    let (env, stl) =
      List.fold ~init:(env, []) stl ~f:(fun (env, stl) st ->
          let (env, st) = stmt env st in
          (* Accumulate statements in reverse order *)
          let stl =
            match st with
            | (_, Aast.Block (_, stl')) -> List.rev stl' @ stl
            | _ -> st :: stl
          in
          (env, stl))
    in
    (env, List.rev stl)

  and stmt env (pos, st) =
    let (env, st) = stmt_ env pos st in
    Typing_debug.log_env_if_too_big pos env;
    (env, (pos, st))

  and stmt_ env pos st =
    (* Type check a loop. f env = (env, result) checks the body of the loop.
     * We iterate over the loop until the "next" continuation environment is
     * stable. alias_depth is supposed to be an upper bound on this; but in
     * certain cases this fails (e.g. a generic type grows unboundedly). TODO:
     * fix this.
     *)
    let infer_loop env f =
      let in_loop_outer = env.in_loop in
      let alias_depth =
        if in_loop_outer then
          1
        else
          Typing_alias.get_depth (pos, st)
      in
      let max_number_of_iterations =
        Option.value
          ~default:alias_depth
          (Env.get_tcopt env |> TCO.loop_iteration_upper_bound)
      in
      let env = { env with in_loop = true } in
      let continuations_converge env old_conts new_conts =
        CMap.for_all2
          ~f:(fun _ old_cont_entry new_cont_entry ->
            Typing_per_cont_ops.is_sub_opt_entry
              SubType.is_sub_type
              env
              new_cont_entry
              old_cont_entry)
          old_conts
          new_conts
      in
      let rec loop env n =
        (* Remember the old environment *)
        let old_conts = env.lenv.per_cont_env in
        let (env, result) = f env in
        let new_conts = env.lenv.per_cont_env in
        (* Finish if we reach the bound, or if the environments match *)
        if
          Int.equal n max_number_of_iterations
          || continuations_converge env old_conts new_conts
        then
          let () = log_iteration_count env pos n in
          let env = { env with in_loop = in_loop_outer } in
          (env, result)
        else
          loop env (n + 1)
      in
      loop env 1
    in
    let env = Env.open_tyvars env pos in
    (fun (env, tb) ->
      let ((env, ty_err_opt), res) =
        (Typing_solver.close_tyvars_and_solve env, tb)
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      (env, res))
    @@
    match st with
    | Fallthrough ->
      let env =
        LEnv.move_and_merge_next_in_cont ~join_pos:pos env C.Fallthrough
      in
      (env, Aast.Fallthrough)
    | Noop -> (env, Aast.Noop)
    | AssertEnv _ -> (env, Aast.Noop)
    | Yield_break ->
      let env = LEnv.move_and_merge_next_in_cont ~join_pos:pos env C.Exit in
      (env, Aast.Yield_break)
    | Expr e ->
      let (env, te, _) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          e
      in
      let env =
        if TFTerm.typed_expression_exits te then
          LEnv.move_and_merge_next_in_cont env ~join_pos:pos C.Exit
        else
          env
      in
      (env, Aast.Expr te)
    | If (e, b1, b2) ->
      let assert_refinement_env =
        assert_env_blk ~pos ~at:`Start Aast.Refinement
      in
      let (env, te, _) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          e
      in
      let (env, tb1, tb2) =
        branch
          ~join_pos:pos
          env
          (fun env ->
            let (env, { lset; pkgs }) = Expr.condition env true te in
            let (env, b1) =
              Env.with_packages env pkgs @@ fun env ->
              let refinement_map = refinement_annot_map env lset in
              let (env, b1) = block env b1 in
              let b1 = assert_refinement_env refinement_map b1 in
              (env, b1)
            in
            let rec get_loaded_packages_from_invariant (_, _, e) acc =
              match e with
              | Aast.Package (_, pkg) -> SSet.add pkg acc
              | Aast.Binop { bop = Ast_defs.Ampamp; lhs; rhs } ->
                get_loaded_packages_from_invariant lhs acc
                |> get_loaded_packages_from_invariant rhs
              | _ -> acc
            in
            (* Since `invariant(cond, msg)` is typed as `if (!cond) { invariant_violation(msg) }`,
               revisit the branch and harvest packages loaded in `cond` if the branch contains an
               invariant statement. *)
            let env =
              List.fold ~init:env b1 ~f:(fun env (_, tst) ->
                  match (te, tst) with
                  | ( (_, _, Aast.Unop (Ast_defs.Unot, e)),
                      Aast.Expr (_, _, Call { func = (_, _, Id (_, s)); _ }) )
                    when String.equal
                           s
                           SN.AutoimportedFunctions.invariant_violation ->
                    get_loaded_packages_from_invariant e SSet.empty
                    |> Env.load_packages env
                  | _ -> env)
            in
            (env, b1))
          (fun env ->
            let (env, { lset; pkgs }) = Expr.condition env false te in
            Env.with_packages env pkgs @@ fun env ->
            let refinement_map = refinement_annot_map env lset in
            let (env, b2) = block env b2 in
            let b2 = assert_refinement_env refinement_map b2 in
            (env, b2))
      in
      (* TODO TAST: annotate with joined types *)
      (env, Aast.If (te, tb1, tb2))
    | Return None ->
      let env = Typing_return.check_inout_return pos env in
      let rty = MakeType.void (Reason.Rwitness pos) in
      let { Typing_env_return_info.return_type = expected_return; _ } =
        Env.get_return env
      in
      let expected_return =
        Typing_return.strip_awaitable (Env.get_fn_kind env) env expected_return
      in
      let env =
        match Env.get_fn_kind env with
        | Ast_defs.FGenerator
        | Ast_defs.FAsyncGenerator ->
          env
        | _ ->
          Typing_return.implicit_return
            env
            pos
            ~expected:expected_return.et_type
            ~actual:rty
            ~hint_pos:None
            ~is_async:false
      in
      let env = LEnv.move_and_merge_next_in_cont ~join_pos:pos env C.Exit in
      (env, Aast.Return None)
    | Return (Some e) ->
      let env = Typing_return.check_inout_return pos env in
      let (_, expr_pos, _) = e in
      let Typing_env_return_info.{ return_type; return_disposable } =
        Env.get_return env
      in
      let return_type =
        Typing_return.strip_awaitable (Env.get_fn_kind env) env return_type
      in
      let expected =
        Some
          (ExpectedTy.make_and_allow_coercion
             expr_pos
             Reason.URreturn
             return_type)
      in
      if return_disposable then enforce_return_disposable env e;
      let (env, te, rty) =
        Expr.expr
          ~expected
          ~config:Expr.Config.default
          ~ctxt:
            Expr.Context.{ default with is_using_clause = return_disposable }
          env
          e
      in
      (* This is a unify_error rather than a return_type_mismatch because the return
       * statement is the problem, not the return type itself. *)
      let (env, ty_err_opt) =
        Typing_coercion.coerce_type
          expr_pos
          Reason.URreturn
          env
          rty
          return_type
          Typing_error.Callback.unify_error
      in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      let ty_mismatch_opt =
        mk_ty_mismatch_opt rty return_type.et_type ty_err_opt
      in
      let env = LEnv.move_and_merge_next_in_cont ~join_pos:pos env C.Exit in
      (env, Aast.Return (Some (hole_on_ty_mismatch ~ty_mismatch_opt te)))
    | Do (b, e) ->
      (* NOTE: leaks scope as currently implemented; this matches
         the behavior in naming (cf. `do_stmt` in naming/naming.ml).
      *)
      let (env, (tb, te)) =
        LEnv.stash_and_do env [C.Continue; C.Break; C.Do] (fun env ->
            let env = LEnv.save_and_merge_next_in_cont ~join_pos:pos env C.Do in
            let (env, _) = block env b in
            (* saving the locals in continue here even if there is no continue
             * statement because they must be merged at the end of the loop, in
             * case there is no iteration *)
            let env =
              LEnv.save_and_merge_next_in_cont ~join_pos:pos env C.Continue
            in
            let (env, tb) =
              infer_loop env (fun env ->
                  let env =
                    LEnv.update_next_from_conts
                      ~join_pos:pos
                      env
                      [C.Continue; C.Next]
                  in
                  (* The following is necessary in case there is an assignment in the
                   * expression *)
                  let (env, te, _) =
                    Expr.expr
                      ~expected:None
                      ~config:Expr.Config.default
                      ~ctxt:Expr.Context.default
                      env
                      e
                  in
                  let (env, { pkgs; _ }) = Expr.condition env true te in
                  Env.with_packages env pkgs @@ fun env ->
                  let env =
                    LEnv.update_next_from_conts ~join_pos:pos env [C.Do; C.Next]
                  in
                  let (env, tb) = block env b in
                  (env, tb))
            in
            let env =
              LEnv.update_next_from_conts ~join_pos:pos env [C.Continue; C.Next]
            in
            let (env, te, _) =
              Expr.expr
                ~expected:None
                ~config:Expr.Config.default
                ~ctxt:Expr.Context.default
                env
                e
            in
            let (env, _) = Expr.condition env false te in
            let env =
              LEnv.update_next_from_conts ~join_pos:pos env [C.Break; C.Next]
            in
            (env, (tb, te)))
      in
      (env, Aast.Do (tb, te))
    | While (e, b) ->
      let (env, (te, tb, refinement_map)) =
        LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
            let env =
              LEnv.save_and_merge_next_in_cont ~join_pos:pos env C.Continue
            in
            let (env, tb) =
              infer_loop env (fun env ->
                  let env =
                    LEnv.update_next_from_conts
                      ~join_pos:pos
                      env
                      [C.Continue; C.Next]
                  in
                  let join_map = annot_map env in
                  (* The following is necessary in case there is an assignment in the
                   * expression *)
                  let (env, te, _) =
                    Expr.expr
                      ~expected:None
                      ~config:Expr.Config.default
                      ~ctxt:Expr.Context.default
                      env
                      e
                  in
                  let (env, { lset; pkgs }) = Expr.condition env true te in
                  Env.with_packages env pkgs @@ fun env ->
                  let refinement_map = refinement_annot_map env lset in
                  (* TODO TAST: avoid repeated generation of block *)
                  let (env, tb) = block env b in

                  (* Annotate loop body with join and refined environments *)
                  let assert_env_blk = assert_env_blk ~pos ~at:`Start in
                  let tb = assert_env_blk Aast.Refinement refinement_map tb in
                  let tb = assert_env_blk Aast.Join join_map tb in

                  (env, tb))
            in
            let env =
              LEnv.update_next_from_conts ~join_pos:pos env [C.Continue; C.Next]
            in
            let (env, te, _) =
              Expr.expr
                ~expected:None
                ~config:Expr.Config.default
                ~ctxt:Expr.Context.default
                env
                e
            in
            let (env, { lset; _ }) = Expr.condition env false te in
            let refinement_map_at_exit = refinement_annot_map env lset in
            let env =
              LEnv.update_next_from_conts env ~join_pos:pos [C.Break; C.Next]
            in
            (env, (te, tb, refinement_map_at_exit)))
      in
      let while_st = Aast.While (te, tb) in
      (* Export the refined environment after the exit condition holds *)
      let while_st =
        assert_env_stmt ~pos ~at:`End Aast.Refinement refinement_map while_st
      in
      (env, while_st)
    | Using
        {
          us_has_await = has_await;
          us_exprs = (loc, using_clause);
          us_block = using_block;
          us_is_block_scoped;
        } ->
      let (env, typed_using_clause, using_vars) =
        Using_stmt.check_using_clause env has_await using_clause
      in
      let (env, typed_using_block) = block env using_block in
      (* Remove any using variables from the environment, as they should not
       * be in scope outside the block *)
      let env = List.fold_left using_vars ~init:env ~f:Env.unset_local in
      ( env,
        Aast.Using
          Aast.
            {
              us_has_await = has_await;
              us_exprs = (loc, typed_using_clause);
              us_block = typed_using_block;
              us_is_block_scoped;
            } )
    | For (e1, e2, e3, b) ->
      let e2 =
        match e2 with
        | Some e2 -> e2
        | None -> ((), Pos.none, True)
      in
      let (env, (te1, te2, te3, tb, refinement_map)) =
        LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
            (* For loops leak their initializer, but nothing that's defined in the
               body
            *)
            let (env, te1, _) =
              Expr.exprs
                ~expected:None
                ~config:Expr.Config.default
                ~ctxt:Expr.Context.default
                env
                e1
            in
            (* initializer *)
            let env =
              LEnv.save_and_merge_next_in_cont ~join_pos:pos env C.Continue
            in
            let (env, (tb, te3)) =
              infer_loop env (fun env ->
                  (* The following is necessary in case there is an assignment in the
                   * expression *)
                  let (env, te2, _) =
                    Expr.expr
                      ~expected:None
                      ~config:Expr.Config.default
                      ~ctxt:Expr.Context.default
                      env
                      e2
                  in
                  let (env, { lset; pkgs }) = Expr.condition env true te2 in
                  Env.with_packages env pkgs @@ fun env ->
                  let refinement_map = refinement_annot_map env lset in
                  let (env, tb) = block env b in
                  let env =
                    LEnv.update_next_from_conts
                      ~join_pos:pos
                      env
                      [C.Continue; C.Next]
                  in
                  let join_map = annot_map env in
                  let (env, te3, _) =
                    Expr.exprs
                      ~expected:None
                      ~config:Expr.Config.default
                      ~ctxt:Expr.Context.default
                      env
                      e3
                  in

                  (* Export the join and refinement environments *)
                  let assert_env_blk = assert_env_blk ~pos ~at:`Start in
                  let tb = assert_env_blk Aast.Refinement refinement_map tb in
                  let tb = assert_env_blk Aast.Join join_map tb in

                  (env, (tb, te3)))
            in
            let env =
              LEnv.update_next_from_conts ~join_pos:pos env [C.Continue; C.Next]
            in
            let (env, te2, _) =
              Expr.expr
                ~expected:None
                ~config:Expr.Config.default
                ~ctxt:Expr.Context.default
                env
                e2
            in
            let (env, { lset; _ }) = Expr.condition env false te2 in
            let refinement_map_at_exit = refinement_annot_map env lset in
            let env =
              LEnv.update_next_from_conts ~join_pos:pos env [C.Break; C.Next]
            in
            (env, (te1, te2, te3, tb, refinement_map_at_exit)))
      in
      let for_st = Aast.For (te1, Some te2, te3, tb) in
      let for_st =
        assert_env_stmt ~pos ~at:`End Aast.Refinement refinement_map for_st
      in
      (env, for_st)
    | Switch (((_, pos, _) as e), cl, dfl) ->
      let (env, te, ty) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          e
      in
      (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
       * See the note in
       * http://php.net/manual/en/control-structures.continue.php *)
      let (env, (te, tcl, tdfl)) =
        LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
            let parent_locals = LEnv.get_all_locals env in
            let (env, tcl, tdfl) = case_list parent_locals ty env pos cl dfl in
            let env =
              LEnv.update_next_from_conts
                ~join_pos:pos
                env
                [C.Continue; C.Break; C.Next]
            in
            (env, (te, tcl, tdfl)))
      in
      (env, Aast.Switch (te, tcl, tdfl))
    | Foreach (e1, e2, b) ->
      (* It's safe to do foreach over a disposable, as no leaking is possible *)
      let (env, te1, ty1) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.{ default with accept_using_var = true }
          ~ctxt:Expr.Context.default
          env
          e1
      in
      let (env, (te1, te2, tb)) =
        LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
            let env =
              LEnv.save_and_merge_next_in_cont ~join_pos:pos env C.Continue
            in
            let (_, p1, _) = e1 in
            let ((env, ty_err_opt), tk, tv, ty_mismatch_opt) =
              as_expr env ty1 p1 e2
            in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            let (env, (te2, tb)) =
              infer_loop env (fun env ->
                  let env =
                    LEnv.update_next_from_conts
                      ~join_pos:pos
                      env
                      [C.Continue; C.Next]
                  in
                  let join_map = annot_map env in
                  let (env, te2) = As_expr.bind_as_expr env p1 tk tv e2 in
                  let (env, tb) = block env b in
                  (* Export the join environment *)
                  let tb =
                    assert_env_blk ~pos ~at:`Start Aast.Join join_map tb
                  in
                  (env, (te2, tb)))
            in
            let env =
              LEnv.update_next_from_conts
                ~join_pos:pos
                env
                [C.Continue; C.Break; C.Next]
            in
            (env, (hole_on_ty_mismatch ~ty_mismatch_opt te1, te2, tb)))
      in
      (env, Aast.Foreach (te1, te2, tb))
    | Try (tb, cl, fb) ->
      let (env, ttb, tcl, tfb) = try_catch ~join_pos:pos env tb cl fb in
      (env, Aast.Try (ttb, tcl, tfb))
    | Concurrent b ->
      let check_expr_rhs env s =
        match s with
        | ( _pos,
            ( Expr ((), _, Binop { bop = Ast_defs.Eq _; lhs = _; rhs = e })
            | Expr (((), _, _) as e) ) ) ->
          let (env, te, ty) =
            Expr.expr
              ~expected:None
              ~config:Expr.Config.default
              ~ctxt:Expr.Context.default
              env
              e
          in
          (env, Some (te, ty))
        | _ -> (env, None)
      in
      let check_assign env ((((pos : Pos.t), s_) as s), te_ty_opt) =
        match s_ with
        | Expr (((), _, Binop { bop = Ast_defs.Eq op; lhs; rhs = _ }) as outer)
          ->
          (match te_ty_opt with
          | Some (te, ty) ->
            let (env, te, _ty) =
              Binop.check_binop
                ~check_defined:true
                ~expected:None
                env
                outer
                pos
                (Ast_defs.Eq op)
                lhs
                (Either.Second (te, ty))
            in
            (env, (pos, Expr te))
          | None ->
            Errors.internal_error pos "Missing type while checking Concurrent";
            stmt env s)
        | Expr _e ->
          (match te_ty_opt with
          | Some (te, _ty) -> (env, (pos, Expr te))
          | None ->
            Errors.internal_error pos "Missing type while checking Concurrent";
            stmt env s)
        | _ -> stmt env s
      in
      let env = might_throw ~join_pos:pos env in
      let (env, te_tyl) = List.map_env env b ~f:check_expr_rhs in
      let (env, b) = List.map_env env (List.zip_exn b te_tyl) ~f:check_assign in
      (env, Aast.Concurrent b)
    | Throw e ->
      let (_, p, _) = e in
      let (env, te, ty) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          e
      in
      let (env, ty_err_opt) = coerce_to_throwable p env ty in
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
      let env = move_and_merge_next_in_catch ~join_pos:pos env in
      (env, Aast.Throw te)
    | Continue ->
      let env = LEnv.move_and_merge_next_in_cont ~join_pos:pos env C.Continue in
      (env, Aast.Continue)
    | Break ->
      let env = LEnv.move_and_merge_next_in_cont ~join_pos:pos env C.Break in
      (env, Aast.Break)
    | Declare_local ((p, lvar), hint, exp) ->
      let hint =
        if TypecheckerOptions.everything_sdt env.genv.tcopt then
          (fst hint, Hlike hint)
        else
          hint
      in
      let ((env, err), hty) =
        Phase.localize_hint_no_subst env ~ignore_errors:false hint
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) err;
      let (is_defined, env, te, ety) =
        match exp with
        | None ->
          (false, env, None, Typing_make_type.nothing (Reason.Rwitness p))
        | Some exp ->
          let (env, te, ety) =
            Expr.expr
              ~expected:None
              ~config:Expr.Config.default
              ~ctxt:Expr.Context.default
              env
              exp
          in
          (true, env, Some te, ety)
      in
      let env = set_valid_rvalue ~is_defined p env lvar (Some hty) ety in
      (env, Aast.Declare_local ((p, lvar), hint, te))
    | Match _ -> failwith "TODO(jakebailey): match statements"
    | Awaitall _
    | Block _
    | Markup _ ->
      failwith
        ("Unexpected nodes in AST. These nodes should have been removed in naming. "
        ^ Pos.string_no_file pos)

  and finally_w_cont fb env ctx =
    (* The only locals in scope are the ones from the current continuation *)
    let env = Env.env_with_locals env @@ CMap.singleton C.Next ctx in
    let (env, _tfb) = block env fb in
    (env, LEnv.get_all_locals env)

  and finally ~join_pos env fb =
    match fb with
    | [] ->
      let env = LEnv.update_next_from_conts ~join_pos env [C.Next; C.Finally] in
      (env, [])
    | _ ->
      let initial_locals = LEnv.get_all_locals env in
      (* First typecheck the finally block against all relevant continuations merged
       * together. The relevant continuations are those corresponding to statements
       * which will cause control flow to jump to the finally block, e.g. `break`.
       * During this phase, record errors found in the finally block, but discard
       * the resulting environment. *)
      let env =
        LEnv.update_next_from_conts
          ~join_pos
          env
          Typing_per_cont_env.continuations_for_finally
      in
      let (env, tfb) = block env fb in
      let env = Env.env_with_locals env initial_locals in
      (* Second, typecheck the finally block once against each continuation. This
       * helps be more clever about what each continuation will be after the
       * finally block.
       * We don't want to record errors during this phase, because certain types
       * of errors will fire wrongly. For example, if $x is nullable in some
       * continuations but not in others, then we must use `?->` on $x, but an
       * error will fire when typechecking the finally block againts continuations
       * where $x is non-null. *)
      let finally_w_cont env _key = finally_w_cont fb env in
      let (env, locals_map) =
        Errors.ignore_ (fun () ->
            CMap.map_env finally_w_cont env initial_locals)
      in
      let union env _key = LEnv.union_contextopts ~join_pos env in
      let (env, locals) = Try.finally_merge union env locals_map in
      (Env.env_with_locals env locals, tfb)

  and try_catch ~join_pos env tb cl fb =
    let parent_locals = LEnv.get_all_locals env in
    (* If any `break`, `continue`, `exit`, etc. happens directly inside the `try`
     * block, they will cause control flow to go to the `finally` block first.
     * I.o.w. the `finally` block is typechecked with the corresponding continuations.
     * Therefore we need to stash the corresponding pre-existing continuations
     * so that we don't typecheck the finally block with those. *)
    let env =
      LEnv.drop_conts env Typing_per_cont_env.continuations_for_finally
    in
    let (env, (ttb, tcb)) =
      Env.in_try env (fun env ->
          let (env, ttb) = block env tb in
          let env = LEnv.move_and_merge_next_in_cont ~join_pos env C.Finally in
          let catchctx = LEnv.get_cont_option env C.Catch in
          let (env, lenvtcblist) = List.map_env env ~f:(catch catchctx) cl in
          let (lenvl, tcb) = List.unzip lenvtcblist in
          let env = LEnv.union_lenv_list ~join_pos env env.lenv lenvl in
          let env = LEnv.move_and_merge_next_in_cont ~join_pos env C.Finally in
          (env, (ttb, tcb)))
    in
    let (env, tfb) = finally ~join_pos env fb in
    let env = LEnv.update_next_from_conts ~join_pos env [C.Finally] in
    let env = LEnv.drop_cont env C.Finally in
    let env =
      LEnv.restore_and_merge_conts_from
        ~join_pos
        env
        parent_locals
        Typing_per_cont_env.continuations_for_finally
    in
    (env, ttb, tcb, tfb)

  and case_list parent_locals ty env switch_pos cl dfl =
    let initialize_next_cont env =
      let env = LEnv.restore_conts_from env parent_locals [C.Next] in
      let env =
        LEnv.update_next_from_conts
          ~join_pos:switch_pos
          env
          [C.Next; C.Fallthrough]
      in
      LEnv.drop_cont env C.Fallthrough
    in
    let check_fallthrough
        env switch_pos case_pos ~next_pos block ~last ~is_default =
      if (not (List.is_empty block)) && not last then
        match LEnv.get_cont_option env C.Next with
        | Some _ ->
          Errors.add_error
            Nast_check_error.(
              to_user_error
              @@
              if is_default then
                Default_fallthrough switch_pos
              else
                Case_fallthrough { switch_pos; case_pos; next_pos })
        | None -> ()
    in
    let env =
      (* below, we try to find out if the switch is exhaustive *)
      let has_default = Option.is_some dfl in
      let ((env, ty_err_opt), ty) =
        (* If it hasn't got a default clause then we need to solve type variables
         * in order to check for an enum *)
        if has_default then
          let (env, ty) = Env.expand_type env ty in
          ((env, None), ty)
        else
          Typing_solver.expand_type_and_solve
            env
            ~description_of_expected:"a value"
            switch_pos
            ty
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      (* leverage that enums are checked for exhaustivity *)
      let is_enum =
        let top_type =
          MakeType.class_type
            Reason.Rnone
            SN.Classes.cHH_BuiltinEnum
            [MakeType.mixed Reason.Rnone]
        in
        SubType.is_sub_type_for_union env ty top_type
      in
      (* register that the runtime may throw in case we cannot prove
         that the switch is exhaustive *)
      if has_default || is_enum then
        env
      else
        might_throw ~join_pos:switch_pos env
    in
    let (env, tcl) =
      let rec case_list env = function
        | [] -> (env, [])
        | (((_, pos, _) as e), b) :: rl ->
          let env = initialize_next_cont env in
          let (env, te, _) =
            Expr.expr
              ~expected:None
              ~config:Expr.Config.default
              ~ctxt:Expr.Context.default
              env
              e
          in
          let (env, tb) = block env b in
          let last = List.is_empty rl && Option.is_none dfl in
          let next_pos =
            match rl with
            | ((_, next_pos, _), _) :: _ -> Some next_pos
            | [] -> None
          in
          check_fallthrough
            env
            switch_pos
            pos
            ~next_pos
            b
            ~last
            ~is_default:false;
          let (env, tcl) = case_list env rl in
          (env, (te, tb) :: tcl)
      in
      case_list env cl
    in
    let (env, tdfl) =
      match dfl with
      | None -> (env, None)
      | Some (pos, b) ->
        let env = initialize_next_cont env in
        let (env, tb) = block env b in
        check_fallthrough
          env
          switch_pos
          pos
          ~next_pos:None
          b
          ~last:true
          ~is_default:true;
        (env, Some (pos, tb))
    in
    (env, tcl, tdfl)

  and catch catchctx env (sid, exn_lvar, b) =
    let env = LEnv.replace_cont env C.Next catchctx in
    let cid = CI sid in
    let ety_p = fst sid in
    let (env, _, _, _) = Class_id.instantiable_cid ety_p env cid [] in
    let (env, _tal, _te, ety) = Class_id.class_expr env [] ((), ety_p, cid) in
    let (env, ty_err_opt) = coerce_to_throwable ety_p env ety in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    let (p, x) = exn_lvar in
    let env = set_valid_rvalue ~is_defined:true p env x None ety in
    let (env, tb) = block env b in
    (env, (env.lenv, (sid, exn_lvar, tb)))
end

and Lambda : sig
  val lambda :
    is_anon:bool ->
    closure_class_name:byte_string option ->
    expected:ExpectedTy.t option ->
    pos ->
    env ->
    Nast.fun_ ->
    Nast.capture_lid list ->
    env * Tast.expr * locl_ty

  val bind_params :
    env ->
    ?can_read_globals:is_variadic ->
    no_auto_likes:is_variadic ->
    contexts option ->
    locl_ty option list ->
    Nast.fun_param list ->
    env * Tast.fun_param list

  val fun_ :
    ?abstract:bool ->
    ?native:bool ->
    ?disable:bool ->
    env ->
    Typing_env_return_info.t ->
    pos ->
    Nast.func_body ->
    Ast_defs.fun_kind ->
    env * Tast.stmt list

  val check_function_dynamically_callable :
    this_class:Decl_provider.Class.t option ->
    Typing_env_types.env ->
    Aast_defs.sid option ->
    Nast.fun_ ->
    Typing_defs.decl_ty option list ->
    Typing_defs.locl_ty ->
    Typing_env_types.env * Tast.fun_param list * Tast.stmt list * Tast.ty
end = struct
  (* Given a localized parameter type and parameter information, infer
   * a type for the parameter default expression (if present) and check that
   * it is a subtype of the parameter type (if present).
   * Set the type of the parameter in the locals environment *)
  let rec bind_param
      env
      ?(immutable = false)
      ?(can_read_globals = false)
      ?(no_auto_likes = false)
      (opt_ty1, param) =
    let (env, param_te, ty1) =
      match param.param_expr with
      | None -> begin
        match opt_ty1 with
        | None ->
          (* If no parameter type has been provided, we assume that an error
           * has already been reported, and generate an error type variable *)
          let (env, err_ty) =
            Env.fresh_type_error_contravariant env param.param_pos
          in
          (env, None, err_ty)
        | Some ty1 -> (env, None, ty1)
      end
      | Some e ->
        let decl_hint =
          Option.map
            ~f:(Decl_hint.hint env.decl_env)
            (hint_of_type_hint param.param_type_hint)
        in
        let enforced =
          match decl_hint with
          | None -> Unenforced
          | Some ty ->
            Typing_enforceability.get_enforcement
              ~this_class:(Env.get_self_class env)
              env
              ty
        in

        let expected =
          match opt_ty1 with
          | None -> None
          | Some ty1 ->
            let ty1_enforced = { et_type = ty1; et_enforced = enforced } in
            ExpectedTy.make_and_allow_coercion_opt
              env
              param.param_pos
              Reason.URparam
              ty1_enforced
        in
        let (env, (te, ty2)) =
          let reason = Reason.Rwitness param.param_pos in
          let pure = MakeType.mixed reason in
          let cap =
            if can_read_globals then
              MakeType.capability reason SN.Capabilities.accessGlobals
            else
              pure
          in
          with_special_coeffects env cap pure @@ fun env ->
          Expr.expr
            ~expected
            ~config:Expr.Config.default
            ~ctxt:Expr.Context.default
            env
            e
          |> triple_to_pair
        in
        Typing_sequencing.sequence_check_expr e;
        let (env, ty1) =
          match opt_ty1 with
          (* Type hint is missing *)
          | None -> (env, ty2)
          (* Otherwise we have an explicit type, and the default expression type
           * must be a subtype *)
          | Some ty1 ->
            (* Under Sound Dynamic, if t is the declared type of the parameter, then we
             * allow the default expression to have any type u such that u <: ~t.
             * If t is enforced, then the parameter is assumed to have that type when checking the body,
             *   because we know that enforcement will ensure this.
             * If t is not enforced, then the parameter is assumed to have type u|t when checking the body,
             * as though the default expression had been assigned conditionally to the parameter.
             *)
            let support_dynamic =
              TCO.enable_sound_dynamic (Env.get_tcopt env)
              && Env.get_support_dynamic_type env
            in
            let like_ty1 =
              if support_dynamic && not no_auto_likes then
                TUtils.make_like env ty1
              else
                ty1
            in
            let (env, ty_err_opt) =
              Typing_coercion.coerce_type
                param.param_pos
                Reason.URhint
                env
                ty2
                { et_type = like_ty1; et_enforced = enforced }
                Typing_error.Callback.parameter_default_value_wrong_type
            in
            Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
            if support_dynamic then
              match enforced with
              | Enforced -> (env, ty1)
              | _ -> Union.union env ty1 ty2
            else
              (env, ty1)
        in
        (env, Some te, ty1)
    in
    let (env, user_attributes) =
      User_attribute.attributes_check_def
        env
        SN.AttributeKinds.parameter
        param.param_user_attributes
    in
    let tparam =
      {
        Aast.param_annotation = Tast.make_expr_annotation param.param_pos ty1;
        Aast.param_type_hint = (ty1, hint_of_type_hint param.param_type_hint);
        Aast.param_is_variadic = param.param_is_variadic;
        Aast.param_pos = param.param_pos;
        Aast.param_name = param.param_name;
        Aast.param_expr = param_te;
        Aast.param_callconv = param.param_callconv;
        Aast.param_readonly = param.param_readonly;
        Aast.param_user_attributes = user_attributes;
        Aast.param_visibility = param.param_visibility;
      }
    in
    let mode = get_param_mode param.param_callconv in
    let out_ty =
      match mode with
      | FPinout ->
        let decl_hint =
          Option.map
            ~f:(Decl_hint.hint env.decl_env)
            (hint_of_type_hint param.param_type_hint)
        in
        let enforced =
          match decl_hint with
          | None -> Unenforced
          | Some ty ->
            Typing_enforceability.get_enforcement
              ~this_class:(Env.get_self_class env)
              env
              ty
        in
        begin
          match enforced with
          | Enforced
            when TCO.enable_sound_dynamic env.genv.tcopt
                 && Env.get_support_dynamic_type env ->
            Some
              (TUtils.make_like
                 ~reason:(Reason.Rpessimised_inout (get_pos ty1))
                 env
                 ty1)
          | _ ->
            (* In implicit SD mode, all inout parameters are pessimised, unless marked <<__NoAutoLikes>> *)
            if TCO.everything_sdt env.genv.tcopt && not no_auto_likes then
              Some
                (TUtils.make_like
                   ~reason:(Reason.Rpessimised_inout (get_pos ty1))
                   env
                   ty1)
            else
              Some ty1
        end
      | _ -> None
    in
    let id = Local_id.make_unscoped param.param_name in
    (* Parameters are not typed locals and so have no bound *)
    let env =
      Env.set_local
        ~is_defined:true
        ~bound_ty:None
        ~immutable
        env
        id
        ty1
        param.param_pos
    in
    let env = Env.set_param env id (ty1, param.param_pos, out_ty) in
    let env =
      if has_accept_disposable_attribute param then
        Env.set_using_var env id
      else
        env
    in
    (env, tparam)

  and bind_params
      env ?(can_read_globals = false) ~no_auto_likes ctxs param_tys params =
    let params_need_immutable = Typing_coeffects.get_ctx_vars ctxs in
    let bind_param_and_check env lty_and_param =
      let (_ty, param) = lty_and_param in
      let name = param.param_name in
      let immutable =
        List.exists ~f:(String.equal name) params_need_immutable
      in
      let (env, fun_param) =
        bind_param ~immutable ~can_read_globals ~no_auto_likes env lty_and_param
      in
      (env, fun_param)
    in
    List.map_env env (List.zip_exn param_tys params) ~f:bind_param_and_check

  and closure_bind_param params (env, t_params) ty : env * Tast.fun_param list =
    match !params with
    | [] ->
      (* This code cannot be executed normally, because the arity is wrong
       * and it will error later. Bind as many parameters as we can and carry
       * on. *)
      (env, t_params)
    | param :: paraml ->
      params := paraml;
      (match hint_of_type_hint param.param_type_hint with
      | Some h ->
        let decl_ty = Decl_hint.hint env.decl_env h in
        (match
           Typing_enforceability.get_enforcement
             ~this_class:(Env.get_self_class env)
             env
             decl_ty
         with
        | Unenforced ->
          Typing_log.log_pessimise_param
            env
            ~is_promoted_property:false
            param.param_pos
            param.param_callconv
            param.param_name
        | Enforced -> ());
        let ((env, ty_err_opt1), h) =
          Phase.localize_no_subst env ~ignore_errors:false decl_ty
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
        let (env, t_param) = bind_param env (Some h, param) in
        (env, t_params @ [t_param])
      | None ->
        let ty =
          mk (Reason.Rlambda_param (param.param_pos, get_reason ty), get_node ty)
        in
        let (env, t_param) = bind_param env (Some ty, param) in
        (env, t_params @ [t_param]))

  and closure_bind_variadic env vparam variadic_ty =
    let ((env, ty_err_opt), ty, pos) =
      match hint_of_type_hint vparam.param_type_hint with
      | None ->
        (* if the hint is missing, use the type we expect *)
        ((env, None), variadic_ty, get_pos variadic_ty)
      | Some hint ->
        let ((env, ty_err_opt1), h) = Phase.localize_hint_for_lambda env hint in
        ((env, ty_err_opt1), h, Pos_or_decl.of_raw_pos vparam.param_pos)
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    let r = Reason.Rvar_param_from_decl pos in
    let arr_values = mk (r, get_node ty) in
    let ty = MakeType.vec r arr_values in
    let (env, t_variadic) = bind_param env (Some ty, vparam) in
    (env, t_variadic)

  and closure_bind_opt_param env param : env =
    match param.param_expr with
    | None ->
      let (env, _) = bind_param env (None, param) in
      env
    | Some default ->
      let (env, _te, ty) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          default
      in
      Typing_sequencing.sequence_check_expr default;
      let (env, _) = bind_param env (Some ty, param) in
      env

  let fun_
      ?(abstract = false)
      ?(native = false)
      ?(disable = false)
      env
      return
      pos
      named_body
      f_kind =
    Env.with_env env (fun env ->
        debug_last_pos := pos;
        let env = Env.set_return env return in
        let (env, tb) =
          if disable then
            let () =
              Errors.internal_error
                pos
                ("Type inference for this function has been disabled by the "
                ^ SN.UserAttributes.uaDisableTypecheckerInternal
                ^ " attribute")
            in
            Stmt.block env []
          else
            Stmt.block env named_body.fb_ast
        in
        Typing_sequencing.sequence_check_block named_body.fb_ast;
        let { Typing_env_return_info.return_type = ret; _ } =
          Env.get_return env
        in
        let has_implicit_return = LEnv.has_next env in
        let has_readonly = Env.get_readonly env in
        let env =
          if (not has_implicit_return) || abstract || native || Env.is_hhi env
          then
            env
          else
            Typing_return.fun_implicit_return env pos ret.et_type f_kind
        in
        let env =
          Env.set_fun_tast_info env Tast.{ has_implicit_return; has_readonly }
        in
        debug_last_pos := Pos.none;
        (env, tb))

  (* Here the body of a function or lambda is typechecked again to ensure it is safe
     * to call it from a dynamic context (eg. under dyn..dyn->dyn assumptions).
     * The code below must be kept in sync with with the fun_def checks.
  *)
  let check_function_dynamically_callable
      ~this_class:_ env f_name f params_decl_ty ret_locl_ty =
    let env = { env with checked = CUnderDynamicAssumptions } in
    Typing_log.log_sd_pass env f.f_span;
    let make_dynamic pos =
      MakeType.dynamic (Reason.Rsupport_dynamic_type pos)
    in
    let dynamic_return_ty = make_dynamic (get_pos ret_locl_ty) in
    let hint_pos =
      match f.f_ret with
      | (_, None) ->
        (match f_name with
        | Some (pos, _) ->
          (* We don't have a return hint, highlight the function name instead. *)
          pos
        | None ->
          (* Anonymos function, just use the position of the whole function. *)
          f.f_span)
      | (_, Some (pos, _)) -> pos
    in
    let dynamic_return_info =
      Typing_return.make_info
        hint_pos
        f.f_fun_kind
        f.f_user_attributes
        env
        (MakeType.unenforced dynamic_return_ty)
    in
    let (env, param_tys) =
      Typing_param.make_param_local_tys
        ~dynamic_mode:true
        ~no_auto_likes:false
        env
        params_decl_ty
        f.f_params
    in
    let (env, dynamic_params) =
      bind_params ~no_auto_likes:false env f.f_ctxs param_tys f.f_params
    in
    let (pos, name) =
      match f_name with
      | Some n -> n
      | None -> (f.f_span, ";anonymous")
    in
    let env = set_tyvars_variance_in_callable env dynamic_return_ty param_tys in
    let disable =
      Naming_attributes.mem
        SN.UserAttributes.uaDisableTypecheckerInternal
        f.f_user_attributes
    in

    let (env, dynamic_body) =
      Errors.try_with_result
        (fun () ->
          fun_ ~disable env dynamic_return_info pos f.f_body f.f_fun_kind)
        (fun env_and_dynamic_body error ->
          if not @@ TCO.everything_sdt env.genv.tcopt then
            Errors.function_is_not_dynamically_callable name error;
          env_and_dynamic_body)
    in
    (env, dynamic_params, dynamic_body, dynamic_return_ty)

  (* Make a type-checking function for an anonymous function or lambda. *)
  (* Here ret_ty should include Awaitable wrapper *)
  (* TODO: ?el is never set; so we need to fix variadic use of lambda *)
  let closure_make
      ?el
      ?ret_ty
      ?(check_escapes = true)
      ~supportdyn
      ~closure_class_name
      env
      lambda_pos
      decl_ft
      f
      ft
      idl
      is_anon =
    let type_closure f =
      (* Wrap the function f so that it can freely clobber function-specific
         parts of the environment; the clobbered parts are restored before
         returning the result. Additionally, we also prevent type parameters
         created in the closure from unsoundly leaking into the environment
         of the enclosing function.

         Also here we wrap `supportdyn<_>` around the function type if
         this is an SDT function.
      *)
      let snap = Typing_escape.snapshot_env env in
      let (env, (escaping, (te, ft, support_dynamic_type))) =
        Env.closure env (fun env ->
            stash_conts_for_closure env lambda_pos is_anon idl (fun env ->
                let (env, res) = f env in
                let escaping = Typing_escape.escaping_from_snapshot snap env in
                (env, (escaping, res))))
      in
      (* After the body of the function is checked, erase all the type parameters
         created from the env and the return type. *)
      let (env, ret_ty) =
        if check_escapes then
          Typing_escape.refresh_env_and_type
            ~remove:escaping
            ~pos:lambda_pos
            env
            ft.ft_ret.et_type
        else
          (env, ft.ft_ret.et_type)
      in
      ( env,
        ( te,
          { ft with ft_ret = { ft.ft_ret with et_type = ret_ty } },
          support_dynamic_type ) )
    in
    type_closure @@ fun env ->
    let nb = f.f_body in
    (* Extract capabilities from AAST and add them to the environment *)
    let (env, capability) =
      match (f.f_ctxs, f.f_unsafe_ctxs) with
      | (None, None) ->
        (* if the closure has no explicit coeffect annotations,
           do _not_ insert (unsafe) capabilities into the environment;
           instead, rely on the fact that a capability from an enclosing
           scope can simply be captured, which has the same semantics
           as redeclaring and shadowing with another same-typed capability.
           This avoid unnecessary overhead in the most common case, i.e.,
           when a closure does not need a different (usually smaller)
           set of capabilities. *)
        ( env,
          (Env.get_local env Typing_coeffects.local_capability_id)
            .Typing_local_types.ty )
      | (_, _) ->
        let (env, cap_ty, unsafe_cap_ty) =
          Typing_coeffects.type_capability env f.f_ctxs f.f_unsafe_ctxs f.f_span
        in
        let (env, _) =
          Typing_coeffects.register_capabilities env cap_ty unsafe_cap_ty
        in
        (env, cap_ty)
    in
    let ft =
      { ft with ft_implicit_params = { capability = CapTy capability } }
    in
    (* Check attributes on the lambda *)
    let (env, user_attributes) =
      User_attribute.attributes_check_def
        env
        SN.AttributeKinds.lambda
        f.f_user_attributes
    in
    (* Regard a lambda as supporting dynamic if
       *   it has the attribute <<__SupportDynamicType>>;
       *   or its enclosing method, function or class has the attribute <<__SupportDynamicType>>;
       *   or it must support dynamic because of the expected type
    *)
    let support_dynamic_type =
      Naming_attributes.mem
        SN.UserAttributes.uaSupportDynamicType
        user_attributes
      || Env.get_support_dynamic_type env
      || supportdyn
    in
    let no_auto_likes =
      Naming_attributes.mem SN.UserAttributes.uaNoAutoLikes user_attributes
      || Env.get_no_auto_likes env
    in
    (* A closure typed in env with parameters p1..pn and captured variables c1..cn
     * roughly compiles to
     *
     * class Closure$name {
     *   private env(c1) c1;
     *   ...
     *   private env(cn) cn;
     *   public static function __invoke(p1, ..., pn) {
     *     /* closure body, references to cn are replaced with $this->cn */
     *   }
     * }
     *
     * To make a class SDT without considering enforcement, we make its properties
     * like types so that they may be written values with type `dynamic`. The following
     * fold analogously makes an SDT closure's captured variables be like types, and
     * elsewhere we skip typing closures entirely when typing the dynamic pass of a function.
     *
     * This is clearly a less precise way to type closures, but it allows us to avoid typing
     * SDT closures 2^depth times.
     *)
    let env =
      if support_dynamic_type then
        (* quiet to avoid duplicate error about capturing an undefined variable *)
        let locals = Env.get_locals ~quiet:true env idl in
        let like_locals =
          Local_id.Map.map
            (fun local ->
              Typing_local_types.
                {
                  local with
                  ty =
                    TUtils.make_like
                      ~reason:(Reason.Rcaptured_like lambda_pos)
                      env
                      local.ty;
                })
            locals
        in
        Env.set_locals env like_locals
      else
        env
    in
    (* Inout parameters should be pessimised in implicit pessimisation mode *)
    let ft =
      if
        support_dynamic_type
        && (not no_auto_likes)
        && TCO.everything_sdt env.genv.tcopt
      then
        {
          ft with
          ft_params =
            List.map ft.ft_params ~f:(fun fp ->
                match get_fp_mode fp with
                | FPinout ->
                  {
                    fp with
                    fp_type =
                      {
                        fp.fp_type with
                        et_type =
                          TUtils.make_like
                            ~reason:(Reason.Rpessimised_inout fp.fp_pos)
                            env
                            fp.fp_type.et_type;
                      };
                  }
                | _ -> fp);
        }
      else
        ft
    in
    let env = Env.clear_params env in
    let non_variadic_ft_params =
      if get_ft_variadic ft then
        List.drop_last_exn ft.ft_params
      else
        ft.ft_params
    in
    let make_variadic_arg env varg tyl =
      let remaining_types =
        (* It's possible the variadic arg will capture the variadic
         * parameter of the supplied arity (if arity is Fvariadic)
         * and additional supplied params.
         *
         * For example in cases such as:
         *  lambda1 = (int $a, string...$c) ==> {};
         *  lambda1(1, "hello", ...$y); (where $y is a variadic string)
         *  lambda1(1, "hello", "world");
         * then ...$c will contain "hello" and everything in $y in the first
         * example, and "hello" and "world" in the second example.
         *
         * To account for a mismatch in arity, we take the remaining supplied
         * parameters and return a list of all their types. We'll use this
         * to create a union type when creating the typed variadic arg.
         *)
        let remaining_params =
          List.drop non_variadic_ft_params (List.length f.f_params - 1)
        in
        List.map ~f:(fun param -> param.fp_type.et_type) remaining_params
      in
      let r = Reason.Rvar_param varg.param_pos in
      let union = Tunion (tyl @ remaining_types) in
      let (env, t_param) = closure_bind_variadic env varg (mk (r, union)) in
      (env, [t_param])
    in
    let (env, t_variadic_params) =
      match
        ( List.find f.f_params ~f:(fun p -> p.param_is_variadic),
          get_ft_variadic ft )
      with
      | (Some arg, true) ->
        make_variadic_arg env arg [(List.last_exn ft.ft_params).fp_type.et_type]
      | (Some arg, false) -> make_variadic_arg env arg []
      | (_, _) -> (env, [])
    in
    let non_variadic_params =
      List.filter f.f_params ~f:(fun p -> not p.param_is_variadic)
    in
    let params = ref non_variadic_params in
    let (env, t_params) =
      List.fold_left
        ~f:(closure_bind_param params)
        ~init:(env, [])
        (List.map non_variadic_ft_params ~f:(fun x -> x.fp_type.et_type))
    in
    let env = List.fold_left ~f:closure_bind_opt_param ~init:env !params in
    let env =
      List.fold_left ~f:closure_check_param ~init:env non_variadic_params
    in
    let env =
      match el with
      | None -> env
      | Some x ->
        let rec iter l1 l2 =
          match (l1, l2) with
          | (_, []) -> ()
          | ([], _) -> ()
          | (x1 :: rl1, (pkx_2, x2) :: rl2) ->
            param_modes ~env x1 x2 pkx_2;
            iter rl1 rl2
        in
        iter non_variadic_ft_params x;
        wfold_left2 Expr.inout_write_back env non_variadic_ft_params x
    in
    let env = Env.set_fn_kind env f.f_fun_kind in
    let decl_ty =
      Option.map ~f:(Decl_hint.hint env.decl_env) (hint_of_type_hint f.f_ret)
    in
    let hint_pos =
      match snd f.f_ret with
      | Some (ret_pos, _) -> ret_pos
      | None -> lambda_pos
    in
    let ety_env =
      empty_expand_env_with_on_error
        (Env.invalid_type_hint_assert_primary_pos_in_current_decl env)
    in
    let ety_env = { ety_env with wildcard_action = Wildcard_fresh_tyvar } in
    let this_class = Env.get_self_class env in
    let params_decl_ty =
      List.map decl_ft.ft_params ~f:(fun { fp_type = { et_type; _ }; _ } ->
          if Typing_defs.is_wildcard et_type then
            None
          else
            Some et_type)
    in
    (* Do we need to re-check the body of the lambda under dynamic assumptions? *)
    let sdt_dynamic_check_required =
      support_dynamic_type
      && not
           (Typing_dynamic.function_parameters_safe_for_dynamic
              ~this_class
              env
              params_decl_ty)
    in
    (* We wrap result type in supportdyn if it's explicit and dynamic check isn't required.
     * This is an easy way to ensure that the return values support dynamic. *)
    let wrap_return_supportdyn =
      support_dynamic_type && not sdt_dynamic_check_required
    in
    let (env, hret) =
      Typing_return.make_return_type
        ~ety_env
        ~this_class
        ~is_toplevel:false
        env
        ~supportdyn:wrap_return_supportdyn
        ~hint_pos
        ~explicit:decl_ty
        ~default:ret_ty
    in
    let ft = { ft with ft_ret = hret } in
    let env =
      Env.set_return
        env
        (Typing_return.make_info hint_pos f.f_fun_kind [] env hret)
    in
    let local_tpenv = Env.get_tpenv env in
    (* Outer pipe variables aren't available in closures. Note that
     * locals are restored by Env.closure after processing the closure
     *)
    let env =
      Env.unset_local env (Local_id.make_unscoped SN.SpecialIdents.dollardollar)
    in
    let sound_dynamic_check_saved_env = env in
    let (env, tb) = Stmt.block env nb.fb_ast in
    let has_implicit_return = LEnv.has_next env in
    let env =
      if not has_implicit_return then
        env
      else
        Typing_return.fun_implicit_return
          env
          lambda_pos
          hret.et_type
          f.f_fun_kind
    in
    (* For an SDT lambda that doesn't have a second check under dynamic assumptions,
     * and with an inferred return type, we must check that the return type supports dynamic *)
    let env =
      if
        support_dynamic_type
        && (not sdt_dynamic_check_required)
        && not wrap_return_supportdyn
      then (
        let (env, ty_err_opt) =
          SubType.sub_type
            ~coerce:(Some Typing_logic.CoerceToDynamic)
            env
            hret.et_type
            (MakeType.dynamic
               (Reason.Rsupport_dynamic_type (Pos_or_decl.of_raw_pos hint_pos)))
          @@ Some (Typing_error.Reasons_callback.unify_error_at hint_pos)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        env
      ) else
        env
    in
    let has_readonly = Env.get_readonly env in
    let env =
      Env.set_fun_tast_info env Tast.{ has_implicit_return; has_readonly }
    in
    let env =
      if sdt_dynamic_check_required then begin
        Fn.ignore
        @@ check_function_dynamically_callable
             ~this_class
             sound_dynamic_check_saved_env
             None
             f
             params_decl_ty
             hret.et_type;
        (* The following modification is independent from the line above that
           does the dynamic check. Check status is changed to dynamic assumptions
           within the call above. Because we don't store alternative lambdas in
           the TAST, we unconditionally overwrite the check status to be under
           normal assumptions here. *)
        { env with checked = Tast.CUnderNormalAssumptions }
      end else
        env
    in
    let tfun_ =
      {
        Aast.f_annotation = Env.save local_tpenv env;
        Aast.f_readonly_this = f.f_readonly_this;
        Aast.f_span = f.f_span;
        Aast.f_ret = (hret.et_type, hint_of_type_hint f.f_ret);
        Aast.f_readonly_ret = f.f_readonly_ret;
        Aast.f_fun_kind = f.f_fun_kind;
        Aast.f_user_attributes = user_attributes;
        Aast.f_body = { Aast.fb_ast = tb };
        Aast.f_ctxs = f.f_ctxs;
        Aast.f_unsafe_ctxs = f.f_unsafe_ctxs;
        Aast.f_params = t_params @ t_variadic_params;
        Aast.f_external = f.f_external;
        Aast.f_doc_comment = f.f_doc_comment;
      }
    in
    let ty = mk (Reason.Rwitness lambda_pos, Tfun ft) in
    let idl_ty =
      List.map idl ~f:(fun (local, lid) -> (local.Typing_local_types.ty, lid))
    in
    let (env, te) =
      TUtils.make_simplify_typed_expr
        env
        lambda_pos
        ty
        (if is_anon then
          Aast.Efun
            {
              ef_fun = tfun_;
              ef_use = idl_ty;
              ef_closure_class_name = closure_class_name;
            }
        else
          Aast.Lfun (tfun_, idl_ty))
    in
    let env = Env.set_tyvar_variance env ty in
    (env, (te, ft, support_dynamic_type))

  let lambda ~is_anon ~closure_class_name ~expected p env f idl =
    (* This is the function type as declared on the lambda itself.
     * If type hints are absent then use Twildcard instead. *)
    let declared_fe = Decl_nast.lambda_decl_in_env env.decl_env f in
    let { fe_type; fe_pos; _ } = declared_fe in
    let (declared_pos, declared_ft) =
      match get_node fe_type with
      | Tfun ft -> (fe_pos, ft)
      | _ -> failwith "Not a function"
    in
    let declared_decl_ft =
      Typing_enforceability.compute_enforced_and_pessimize_fun_type
        ~this_class:(Env.get_self_class env)
        env
        declared_ft
    in
    (* When creating a closure, the 'this' type will mean the late bound type
     * of the current enclosing class
     *)
    let ety_env =
      empty_expand_env_with_on_error
        (Env.invalid_type_hint_assert_primary_pos_in_current_decl env)
    in
    (* For Twildcard types, generate fresh type variables *)
    let ety_env = { ety_env with wildcard_action = Wildcard_fresh_tyvar } in
    let ((env, ty_err_opt), declared_ft) =
      Phase.(
        localize_ft
          ~instantiation:
            { use_name = "lambda"; use_pos = p; explicit_targs = [] }
          ~ety_env
          ~def_pos:declared_pos
          env
          declared_decl_ft)
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    let idl =
      List.map idl ~f:(fun ((), ((_, name) as id)) ->
          (* Check that none of the explicit capture variables are from a using clause *)
          check_escaping_var env id;

          let ty = Env.get_local env name in
          (ty, id))
    in

    (* Ensure that variadic parameter has a name *)
    (if get_ft_variadic declared_ft then
      match List.last declared_ft.ft_params with
      | Some { fp_name = None; _ } ->
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Ellipsis_strict_mode { require = `Param_name; pos = p })
      | _ -> ());

    (* Is the return type declared? *)
    let is_explicit = Option.is_some (hint_of_type_hint f.f_ret) in
    let check_body_under_known_params ~supportdyn env ?ret_ty ft :
        env * _ * locl_ty =
      let (env, (tefun, ft, support_dynamic_type)) =
        closure_make
          ~supportdyn
          ~closure_class_name
          ?ret_ty
          env
          p
          declared_decl_ft
          f
          ft
          idl
          is_anon
      in
      let ty = mk (Reason.Rwitness p, Tfun ft) in
      let ty =
        if support_dynamic_type then
          MakeType.supportdyn (Reason.Rwitness p) ty
        else
          ty
      in
      (env, tefun, ty)
    in
    let (env, eexpected) =
      expand_expected_and_get_node
        ~strip_supportdyn:true
        ~pessimisable_builtin:true
        env
        expected
    in
    match eexpected with
    | _
    (* When we're in a dynamic pass of a function (or closure) don't type inner closures
     * at all, simply assign them a dynamic type.
     * Exception: we're checking an expression tree. Even in the dynamic pass,
     * maketree_with_type_param needs the lambda to be typed precisely.
     *)
      when Tast.is_under_dynamic_assumptions env.checked
           && not (Env.is_in_expr_tree env) ->
      make_result env p Aast.Omitted (MakeType.dynamic (Reason.Rwitness p))
    | Some (_pos, _ur, supportdyn, ty, Tfun expected_ft) ->
      (* First check that arities match up *)
      check_lambda_arity env p (get_pos ty) declared_ft expected_ft;
      (* Use declared types for parameters in preference to those determined
       * by the context (expected parameters): they might be more general. *)
      let rec replace_non_declared_types
          declared_decl_ft_params declared_ft_params expected_ft_params =
        match
          (declared_decl_ft_params, declared_ft_params, expected_ft_params)
        with
        | ( declared_decl_ft_param :: declared_decl_ft_params,
            declared_ft_param :: declared_ft_params,
            expected_ft_param :: expected_ft_params ) ->
          let rest =
            replace_non_declared_types
              declared_decl_ft_params
              declared_ft_params
              expected_ft_params
          in
          (* If the type parameter did not have a type hint, it is Twildcard and
             we use the expected type instead. Otherwise, declared type takes
             precedence. *)
          let resolved_ft_param =
            if Typing_defs.is_wildcard declared_decl_ft_param.fp_type.et_type
            then
              { declared_ft_param with fp_type = expected_ft_param.fp_type }
            else
              declared_ft_param
          in
          resolved_ft_param :: rest
        | (_, _, []) ->
          (* Morally, this case should match on ([],[]) because we already
             check arity mismatch between declared and expected types. We
             handle it more generally here to be graceful. *)
          declared_ft_params
        | ([], [], _) ->
          if (not (get_ft_variadic declared_ft)) && get_ft_variadic expected_ft
          then
            []
          else
            (* This means the expected_ft params list can have more parameters
             * than declared parameters in the lambda. For variadics, this is OK.
             *)
            expected_ft_params
        | _ ->
          failwith
            "declared_decl_ft_params length not same as declared_ft_params"
      in
      let expected_ft =
        {
          expected_ft with
          ft_params =
            replace_non_declared_types
              declared_decl_ft.ft_params
              declared_ft.ft_params
              expected_ft.ft_params;
          ft_implicit_params = declared_ft.ft_implicit_params;
          ft_flags = declared_ft.ft_flags;
        }
      in
      (* Don't bother passing in `void` if there is no explicit return *)
      let ret_ty =
        match get_node expected_ft.ft_ret.et_type with
        | Tprim Tvoid when not is_explicit -> None
        | _ -> Some expected_ft.ft_ret.et_type
      in
      Typing_log.increment_feature_count env FL.Lambda.contextual_params;
      check_body_under_known_params ~supportdyn env ?ret_ty expected_ft
    | _ ->
      (* If all parameters are annotated with explicit types, then type-check
       * the body under those assumptions and pick up the result type *)
      let all_explicit_params =
        List.for_all f.f_params ~f:(fun param ->
            Option.is_some (hint_of_type_hint param.param_type_hint))
      in
      if all_explicit_params then (
        Typing_log.increment_feature_count
          env
          (if List.is_empty f.f_params then
            FL.Lambda.no_params
          else
            FL.Lambda.explicit_params);
        check_body_under_known_params ~supportdyn:false env declared_ft
      ) else (
        match eexpected with
        | Some (_pos, _ur, supportdyn, expected_ty, _)
          when TUtils.is_mixed env expected_ty
               || is_nonnull expected_ty
               || is_dynamic expected_ty ->
          (* If the expected type of a lambda is mixed, nonnull or dynamic,
           * or supportdyn of these, we construct a function type where the
           * undeclared parameters are set to mixed (when expected type of
           * lambda is mixed or nonnull), or dynamic (when expected type of
           * lambda is dynamic). Wrap mixed or nonnull in supportdyn if the
           * expected type is supportdyn.
           * We leave the return type alone, inferring it if necessary.
           *
           * For an expected mixed/nonnull type, one could argue that the lambda
           * doesn't even need to be checked as it can't be called (there is
           * no downcast to function type). Thus, we should be using nothing
           * to type the arguments. But generally users are very confused by
           * the use of nothing and would expect the lambda body to be
           * checked as though it could be called.
           *)
          let r = Reason.Rwitness p in
          let param_type =
            if is_dynamic expected_ty then
              MakeType.dynamic r
            else if supportdyn then
              MakeType.supportdyn r (MakeType.mixed r)
            else
              MakeType.mixed r
          in
          let replace_non_declared_type declared_decl_ft_param declared_ft_param
              =
            let is_undeclared =
              Typing_defs.is_wildcard declared_decl_ft_param.fp_type.et_type
            in
            if is_undeclared then
              let enforced_ty =
                { et_enforced = Unenforced; et_type = param_type }
              in
              { declared_ft_param with fp_type = enforced_ty }
            else
              declared_ft_param
          in
          let expected_ft =
            let ft_params =
              List.map2_exn
                ~f:replace_non_declared_type
                declared_decl_ft.ft_params
                declared_ft.ft_params
            in
            { declared_ft with ft_params }
          in
          check_body_under_known_params ~supportdyn env expected_ft
        | _ ->
          (* Missing types in parameter and result will have been replaced by fresh type variables *)
          Typing_log.increment_feature_count env FL.Lambda.fresh_tyvar_params;
          let declared_ty = mk (Reason.Rnone, Tfun declared_ft) in
          let supportdyn = Env.get_support_dynamic_type env in
          let declared_ty =
            if supportdyn then
              MakeType.supportdyn Reason.Rnone declared_ty
            else
              declared_ty
          in
          let env = check_expected_ty "Lambda" env declared_ty expected in
          let env = Env.set_tyvar_variance env declared_ty in
          (* TODO(jjwu): the declared_ft here is set to public,
             but is actually inferred from the surrounding context
             (don't think this matters in practice, since we check lambdas separately) *)
          check_body_under_known_params
            ~supportdyn
            env
            ~ret_ty:declared_ft.ft_ret.et_type
            declared_ft
      )
end

and User_attribute : sig
  val attributes_check_def :
    env ->
    byte_string ->
    Nast.user_attribute list ->
    env * (locl_ty, saved_env) Aast_defs.user_attribute list
end = struct
  let user_attribute env Aast.{ ua_name; ua_params } =
    let (env, typed_ua_params) =
      List.map_env env ua_params ~f:(fun env e ->
          let (env, te, _) =
            Expr.expr
              ~expected:None
              ~config:Expr.Config.default
              ~ctxt:Expr.Context.{ default with is_attribute_param = true }
              env
              e
          in
          (env, te))
    in
    (env, Aast.{ ua_name; ua_params = typed_ua_params })

  let attributes_check_def env kind attrs =
    let new_object attr_pos env attr_cid params =
      (* We'd like the class id's position here to be the use position of the attribute
         instead of the decl position of the class *)
      let (_decl_pos, cid_) = attr_cid in
      let (env, _, _, _, _, _, _, _) =
        Expr.new_object
          ~expected:None
          ~check_parent:false
          ~check_not_abstract:false
          ~is_using_clause:false
          attr_pos
          env
          ((), attr_pos, Aast.CI (attr_pos, cid_))
          []
          (List.map ~f:(fun e -> (Ast_defs.Pnormal, e)) params)
          (* list of attr parameter literals *)
          None
        (* no variadic arguments *)
      in
      env
    in
    let env = Typing_attributes.check_def env new_object kind attrs in
    let (env, user_attrs) = List.map_env env attrs ~f:user_attribute in
    (* If NoAutoDynamic is set, disable everything_sdt *)
    let env =
      if Naming_attributes.mem SN.UserAttributes.uaNoAutoDynamic user_attrs then
        Env.set_everything_sdt env false
      else
        env
    in
    (env, user_attrs)
end

and Class_id : sig
  (** Resolve class expressions:
  *     self    CIself       lexically enclosing class
  *     parent  CIparent     lexically enclosing `extends` class
  *     static  CIstatic     late-static-bound class (i.e. runtime receiver)
  *     <id>    CI id        literal class name
  *     <expr>  CIexpr expr  expression that evaluates to an object or classname
  *)
  val class_expr :
    ?check_targs_well_kinded:is_variadic ->
    ?is_attribute_param:is_variadic ->
    ?exact:exact ->
    ?check_explicit_targs:is_variadic ->
    env ->
    Nast.targ list ->
    Nast.class_id ->
    env * Tast.targ list * Tast.class_id * locl_ty

  val class_id_for_new :
    exact:exact ->
    pos ->
    env ->
    (unit, unit) class_id_ ->
    Nast.targ list ->
    newable_class_info

  (* When invoking a method, the class_id is used to determine what class we
   * lookup the method in, but the type of 'this' will be the late bound type.
   * For example:
   *
   *  class C {
   *    public static function get(): this { return new static(); }
   *
   *    public static function alias(): this { return self::get(); }
   *  }
   *
   *  In C::alias, when we invoke self::get(), 'self' is resolved to the class
   *  in the lexical scope (C), so call C::get. However the method is executed in
   *  the current context, so static inside C::get will be resolved to the late
   *  bound type (get_called_class() within C::alias).
   *
   *  This means when determining the type of this, CIparent and CIself should be
   *  changed to CIstatic. For the other cases of C::get() or $c::get(), we only
   *  look at the left hand side of the '::' and use the type associated
   *  with it.
   *
   *  Thus C::get() will return a type C, while $c::get() will return the same
   *  type as $c.
   *)
  val this_for_method : env -> Nast.class_id -> locl_ty -> env * locl_ty

  (** Get class infos for a class expression (e.g. `parent`, `self` or
    regular classnames) - which might resolve to a union or intersection
    of classes - and check they are instantiable.

    FIXME: we need to separate our instantiability into two parts. Currently,
    all this function is doing is checking if a given type is inhabited --
    that is, whether there are runtime values of type Aast. However,
    instantiability should be the stricter notion that T has a runtime
    constructor; that is, `new T()` should be valid. In particular, interfaces
    are inhabited, but not instantiable.
    To make this work with classname, we likely need to add something like
    concrete_classname<T>, where T cannot be an interface. *)
  val instantiable_cid :
    ?exact:exact ->
    pos ->
    env ->
    Nast.class_id_ ->
    Nast.targ list ->
    newable_class_info
end = struct
  let class_expr
      ?(check_targs_well_kinded = false)
      ?(is_attribute_param = false)
      ?(exact = nonexact)
      ?(check_explicit_targs = false)
      (env : env)
      (tal : Nast.targ list)
      ((_, p, cid_) : Nast.class_id) :
      env * Tast.targ list * Tast.class_id * locl_ty =
    let make_result env tal te ty = (env, tal, (ty, p, te), ty) in
    match cid_ with
    | CIparent ->
      (match Env.get_self_id env with
      | Some self ->
        (match Env.get_class env self with
        | Some trait when Ast_defs.is_c_trait (Cls.kind trait) ->
          (match trait_most_concrete_req_class trait env with
          | None ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(primary @@ Primary.Parent_in_trait p);
            let (env, ty) = Env.fresh_type_error env p in
            make_result env [] Aast.CIparent ty
          | Some (_, parent_ty) ->
            (* inside a trait, parent is SN.Typehints.this, but with the
             * type of the most concrete class that the trait has
             * "require extend"-ed *)
            let ((env, ty_err_opt), parent_ty) =
              Phase.localize_no_subst env ~ignore_errors:true parent_ty
            in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            make_result env [] Aast.CIparent parent_ty)
        | _ -> begin
          match Env.get_parent_ty env with
          | None ->
            Typing_error_utils.add_typing_error
              ~env
              Typing_error.(primary @@ Primary.Parent_undefined p);
            let (env, ty) = Env.fresh_type_error env p in
            make_result env [] Aast.CIparent ty
          | Some parent ->
            let ((env, ty_err_opt), parent) =
              Phase.localize_no_subst env ~ignore_errors:true parent
            in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            (* parent is still technically the same object. *)
            make_result env [] Aast.CIparent parent
        end)
      | None -> begin
        match Env.get_parent_ty env with
        | None ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(primary @@ Primary.Parent_undefined p);
          let (env, ty) = Env.fresh_type_error env p in
          make_result env [] Aast.CIparent ty
        | Some parent ->
          let ((env, ty_err_opt), parent) =
            Phase.localize_no_subst env ~ignore_errors:true parent
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          (* parent is still technically the same object. *)
          make_result env [] Aast.CIparent parent
      end)
    | CIstatic ->
      let this =
        if Option.is_some (Env.next_cont_opt env) then
          MakeType.this (Reason.Rwitness p)
        else
          MakeType.nothing (Reason.Rwitness p)
      in
      make_result env [] Aast.CIstatic this
    | CIself ->
      let (env, ty) =
        match Env.get_self_class_type env with
        | Some (c, _, tyl) ->
          (env, mk (Reason.Rwitness p, Tclass (c, exact, tyl)))
        | None ->
          (* Naming phase has already checked and replaced CIself with CI if outside a class *)
          Errors.internal_error p "Unexpected CIself";
          Env.fresh_type_error env p
      in
      make_result env [] Aast.CIself ty
    | CI ((p, id) as c) -> begin
      match Env.get_pos_and_kind_of_generic env id with
      | Some (def_pos, kind) ->
        let simple_kind = Typing_kinding_defs.Simple.from_full_kind kind in
        let param_nkinds =
          Typing_kinding_defs.Simple.get_named_parameter_kinds simple_kind
        in
        let ((env, ty_err_opt), tal) =
          Phase.localize_targs_with_kinds
            ~check_well_kinded:check_targs_well_kinded
            ~is_method:true
            ~def_pos
            ~use_pos:p
            ~use_name:(strip_ns (snd c))
            ~check_explicit_targs
            env
            param_nkinds
            (List.map ~f:snd tal)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        let r = Reason.Rhint (Pos_or_decl.of_raw_pos p) in
        let type_args = List.map tal ~f:fst in
        let tgeneric = MakeType.generic ~type_args r id in
        make_result env tal (Aast.CI c) tgeneric
      | None ->
        (* Not a type parameter *)
        let class_ = Env.get_class env id in
        (match class_ with
        | None ->
          let (env, ty) = Env.fresh_type_error env p in
          make_result env [] (Aast.CI c) ty
        | Some class_ ->
          if not is_attribute_param then
            Option.iter
              ~f:(Typing_error_utils.add_typing_error ~env)
              (TVis.check_top_level_access
                 ~in_signature:false
                 ~use_pos:p
                 ~def_pos:(Cls.pos class_)
                 env
                 (Cls.internal class_)
                 (Cls.get_module class_));
          (* Don't add Exact superfluously to class type if it's final *)
          let exact =
            if Cls.final class_ then
              nonexact
            else
              exact
          in
          let ((env, ty_err_opt), ty, tal) =
            List.map ~f:snd tal
            |> Phase.localize_targs_and_check_constraints
                 ~exact
                 ~check_well_kinded:check_targs_well_kinded
                 ~def_pos:(Cls.pos class_)
                 ~use_pos:p
                 ~check_explicit_targs
                 env
                 c
                 (Reason.Rwitness (fst c))
                 (Cls.tparams class_)
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          make_result env tal (Aast.CI c) ty)
    end
    | CIexpr ((_, p, _) as e) ->
      let (env, te, ty) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          e
      in
      let fold_errs errs =
        let rec aux = function
          | (Ok xs, Ok x :: rest) -> aux (Ok (x :: xs), rest)
          | (Ok xs, Error (x, y) :: rest) -> aux (Error (x :: xs, y :: xs), rest)
          | (Error (xs, ys), Ok x :: rest) ->
            aux (Error (x :: xs, x :: ys), rest)
          | (Error (xs, ys), Error (x, y) :: rest) ->
            aux (Error (x :: xs, y :: ys), rest)
          | (acc, []) -> acc
        in
        aux (Ok [], errs)
      in
      let rec resolve_ety env ty =
        let ((env, ty_err1), ty) =
          Typing_solver.expand_type_and_solve
            ~description_of_expected:"an object"
            env
            p
            ty
        in
        let base_ty = TUtils.get_base_type env ty in
        match deref base_ty with
        | (_, Tnewtype (classname, [the_cls], as_ty))
          when String.equal classname SN.Classes.cClassname ->
          let ((env, ty_err2), (ty, err_res)) = resolve_ety env the_cls in
          let wrap ty = mk (Reason.none, Tnewtype (classname, [ty], as_ty)) in
          let err_res =
            match err_res with
            | Ok ty -> Ok (wrap ty)
            | Error (ty_act, ty_exp) -> Error (wrap ty_act, wrap ty_exp)
          in
          ( (env, Option.merge ty_err1 ty_err2 ~f:Typing_error.both),
            (ty, err_res) )
        | (_, Tgeneric _)
        | (_, Tclass _) ->
          ((env, ty_err1), (ty, Ok ty))
        | (r, Tunion tyl) ->
          let ((env, ty_err2), res) =
            List.map_env_ty_err_opt
              env
              tyl
              ~combine_ty_errs:Typing_error.union_opt
              ~f:resolve_ety
          in
          let (tyl, errs) = List.unzip res in
          let ty = MakeType.union r tyl in
          let err =
            match fold_errs errs with
            | Ok _ -> Ok ty
            | Error (ty_actuals, ty_expects) ->
              let ty_actual = MakeType.union Reason.none ty_actuals
              and ty_expect = MakeType.union Reason.none ty_expects in
              Error (ty_actual, ty_expect)
          in
          ((env, Option.merge ty_err1 ty_err2 ~f:Typing_error.both), (ty, err))
        | (r, Tintersection tyl) ->
          let ((env, ty_err2), res) =
            TUtils.run_on_intersection_with_ty_err env tyl ~f:resolve_ety
          in
          let (tyl, errs) = List.unzip res in
          let (env, ty) = Inter.intersect_list env r tyl in
          let (env, err) =
            match fold_errs errs with
            | Ok _ -> (env, Ok ty)
            | Error (ty_actuals, ty_expects) ->
              let (env, ty_actual) =
                Inter.intersect_list env Reason.none ty_actuals
              in
              let (env, ty_expect) =
                Inter.intersect_list env Reason.none ty_expects
              in
              (env, Error (ty_actual, ty_expect))
          in
          ((env, Option.merge ty_err1 ty_err2 ~f:Typing_error.both), (ty, err))
        | (_, Tdynamic) -> ((env, None), (base_ty, Ok base_ty))
        | (r, Tvar _) ->
          let ty_err2 =
            if TUtils.is_tyvar_error env base_ty then
              None
            else
              Some
                Typing_error.(
                  primary
                  @@ Primary.Unknown_type
                       {
                         expected = "an object";
                         pos = p;
                         reason = lazy (Reason.to_string "It is unknown" r);
                       })
          in
          let (env, ty) = Env.fresh_type_error env p in
          ((env, Option.merge ty_err1 ty_err2 ~f:Typing_error.both), (ty, Ok ty))
        | (_, Tunapplied_alias _) ->
          Typing_defs.error_Tunapplied_alias_in_illegal_context ()
        (* We allow a call through a string in dynamic check mode, as string <:D dynamic *)
        | (r, Tprim Tstring) when Tast.is_under_dynamic_assumptions env.checked
          ->
          ((env, None), (MakeType.dynamic r, Ok (MakeType.dynamic r)))
        | ( r,
            ( Tany _ | Tnonnull | Tvec_or_dict _ | Toption _ | Tprim _ | Tfun _
            | Ttuple _ | Tnewtype _ | Tdependent _ | Tshape _ | Taccess _
            | Tneg _ ) ) ->
          let ty_err2 =
            Some
              Typing_error.(
                primary
                @@ Primary.Expected_class
                     {
                       suffix =
                         Some
                           (lazy
                             (", but got " ^ Typing_print.error env base_ty));
                       pos = p;
                     })
          in
          let ty_nothing = MakeType.nothing Reason.none in
          let (env, ty) =
            if TCO.enable_sound_dynamic (Env.get_tcopt env) then
              (env, MakeType.dynamic r)
            else
              Env.fresh_type_error env p
          in
          let ty_expect = MakeType.classname Reason.none [ty_nothing] in
          ( (env, Option.merge ty_err1 ty_err2 ~f:Typing_error.both),
            (ty, Error (base_ty, ty_expect)) )
      in
      let ((env, ty_err_opt), (result_ty, err_res)) = resolve_ety env ty in
      let ty_mismatch_opt =
        Result.fold
          err_res
          ~ok:(fun _ -> None)
          ~error:(fun (ty_act, ty_expect) -> Some (ty_act, ty_expect))
      in
      let te = hole_on_ty_mismatch ~ty_mismatch_opt te in
      let x = make_result env [] (Aast.CIexpr te) result_ty in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      x

  let class_id_for_new
      ~exact p env (cid : Nast.class_id_) (explicit_targs : Nast.targ list) :
      newable_class_info =
    let (env, tal, te, cid_ty) =
      class_expr
        ~check_targs_well_kinded:true
        ~check_explicit_targs:true
        ~exact
        env
        explicit_targs
        ((), p, cid)
    in
    (* Need to deal with union case *)
    let rec get_info res tyl =
      match tyl with
      | [] -> (env, tal, te, res)
      | ty :: tyl ->
        (match get_node ty with
        | Tunion tyl'
        | Tintersection tyl' ->
          get_info res (tyl' @ tyl)
        | _ ->
          (* Instantiation on an abstract class (e.g. from classname<T>) is
           * via the base type (to check constructor args), but the actual
           * type `ty` must be preserved. *)
          (match get_node (TUtils.get_base_type env ty) with
          | Tdynamic -> get_info (`Dynamic :: res) tyl
          | Tclass (sid, _, _) ->
            let class_ = Env.get_class env (snd sid) in
            (match class_ with
            | None -> get_info res tyl
            | Some class_info ->
              (match (te, cid_ty) with
              (* When computing the classes for a new T() where T is a generic,
               * the class must be consistent (final, final constructor, or
               * <<__ConsistentConstruct>>) for its constructor to be considered *)
              | ((_, _, Aast.CI (_, c)), ty) when is_generic_equal_to c ty ->
                (* Only have this choosing behavior for new T(), not all generic types
                 * i.e. new classname<T>, TODO: T41190512 *)
                if Cls.valid_newable_class class_info then
                  get_info (`Class (sid, class_info, ty) :: res) tyl
                else
                  get_info res tyl
              | _ -> get_info (`Class (sid, class_info, ty) :: res) tyl))
          | _ -> get_info res tyl))
    in
    get_info [] [cid_ty]

  (* When invoking a method, the class_id is used to determine what class we
   * lookup the method in, but the type of 'this' will be the late bound type.
   * For example:
   *
   *  class C {
   *    public static function get(): this { return new static(); }
   *
   *    public static function alias(): this { return self::get(); }
   *  }
   *
   *  In C::alias, when we invoke self::get(), 'self' is resolved to the class
   *  in the lexical scope (C), so call C::get. However the method is executed in
   *  the current context, so static inside C::get will be resolved to the late
   *  bound type (get_called_class() within C::alias).
   *
   *  This means when determining the type of this, CIparent and CIself should be
   *  changed to CIstatic. For the other cases of C::get() or $c::get(), we only
   *  look at the left hand side of the '::' and use the type associated
   *  with it.
   *
   *  Thus C::get() will return a type C, while $c::get() will return the same
   *  type as $c.
   *)
  let this_for_method env (_, p, cid) default_ty =
    match cid with
    | CIparent
    | CIself
    | CIstatic ->
      let (env, _tal, _te, ty) = class_expr env [] ((), p, CIstatic) in
      ExprDepTy.make env ~cid:CIstatic ty
    | _ -> (env, default_ty)

  and instantiable_cid ?(exact = nonexact) p env cid explicit_targs :
      newable_class_info =
    let (env, tal, te, classes) =
      class_id_for_new ~exact p env cid explicit_targs
    in
    List.iter classes ~f:(function
        | `Dynamic -> ()
        | `Class ((pos, name), class_info, c_ty) ->
          let pos = Pos_or_decl.unsafe_to_raw_pos pos in
          let kind = Cls.kind class_info in
          if
            Ast_defs.is_c_trait kind
            || Ast_defs.is_c_enum kind
            || Ast_defs.is_c_enum_class kind
          then
            match cid with
            | CIexpr _
            | CI _ ->
              uninstantiable_error env p cid (Cls.pos class_info) name pos c_ty
            | CIstatic
            | CIparent
            | CIself ->
              ()
          else if Ast_defs.is_c_abstract kind && Cls.final class_info then
            uninstantiable_error env p cid (Cls.pos class_info) name pos c_ty
          else
            ());
    (env, tal, te, classes)
end

and Binop : sig
  (* Type check a binop, where e1 is the lhs and e2 is the rhs sub-exression.
     In case we've already checked e2, allow it to be passed in as it's TAST and type
  *)
  val check_binop :
    check_defined:bool ->
    expected:ExpectedTy.t option ->
    env ->
    Nast.expr ->
    pos ->
    Ast_defs.bop ->
    Nast.expr ->
    (Nast.expr, Tast.expr * locl_ty) Either.t ->
    env * Tast.expr * locl_ty
end = struct
  (* When typing compound assignments we generate a 'fake' expression which
     desugars it to the operation on the rhs of the assignment. If there
     was a subtyping error, we end up with the Hole on the fake rhs
     rather than the original rhs. This function rewrites the
     desugared expression with the Hole in the correct place *)
  let resugar_binop expr =
    match expr with
    | ( topt,
        p,
        Aast.(
          Binop
            {
              bop = _;
              lhs = te1;
              rhs =
                ( _,
                  _,
                  Hole
                    ( (_, _, Binop { bop = op; lhs = _; rhs = te2 }),
                      ty_have,
                      ty_expect,
                      source ) );
            }) ) ->
      let hte2 = mk_hole te2 ~ty_have ~ty_expect ~source in
      let te =
        Aast.Binop { bop = Ast_defs.Eq (Some op); lhs = te1; rhs = hte2 }
      in
      Some (topt, p, te)
    | ( topt,
        p,
        Aast.Binop
          {
            bop = _;
            lhs = te1;
            rhs = (_, _, Aast.Binop { bop = op; lhs = _; rhs = te2 });
          } ) ->
      let te =
        Aast.Binop { bop = Ast_defs.Eq (Some op); lhs = te1; rhs = te2 }
      in
      Some (topt, p, te)
    | _ -> None

  let check_binop ~check_defined ~expected env outer p bop e1 e2 =
    let check_e2 checker env e2 =
      match e2 with
      | Either.First expr -> checker env expr
      | Either.Second (te, ty) -> (env, te, ty)
    in
    match bop with
    | Ast_defs.QuestionQuestion ->
      let (env, te1, ty1) =
        Expr.raw_expr
          ~expected:None
          ~config:Expr.Config.{ default with check_defined }
          ~ctxt:Expr.Context.{ default with lhs_of_null_coalesce = true }
          env
          e1
      in
      let parent_lenv = env.lenv in
      let lenv1 = env.lenv in
      let (env, te2, ty2) =
        check_e2
          (Expr.expr
             ~expected
             ~config:Expr.Config.{ default with check_defined }
             ~ctxt:Expr.Context.default)
          env
          e2
      in
      let lenv2 = env.lenv in
      let env = LEnv.union_lenvs ~join_pos:p env parent_lenv lenv1 lenv2 in
      (* There are two cases: either the left argument was null in which case we
         evaluate the second argument which gets as ty2, or that ty1 wasn't null.
         The following intersection adds the nonnull information. *)
      let (env, ty1) =
        Inter.intersect env ~r:Reason.Rnone ty1 (MakeType.nonnull Reason.Rnone)
      in
      let (env, ty) = Union.union ~approx_cancel_neg:true env ty1 ty2 in
      make_result
        env
        p
        (Aast.Binop { bop = Ast_defs.QuestionQuestion; lhs = te1; rhs = te2 })
        ty
    | Ast_defs.Eq op_opt ->
      let make_result env p te ty =
        let (env, te, ty) = make_result env p te ty in
        let env = Typing_local_ops.check_assignment env te in
        (env, te, ty)
      in
      (match op_opt with
      (* For example, e1 += e2. This is typed and translated as if
       * written e1 = e1 + e2.
       * TODO TAST: is this right? e1 will get evaluated more than once
       *)
      | Some op ->
        let (_, _, expr_) = e1 in
        (match (op, expr_) with
        | (Ast_defs.QuestionQuestion, Class_get _) ->
          Errors.experimental_feature
            p
            "null coalesce assignment operator with static properties";
          expr_error env p outer
        | _ ->
          let (env, te, ty) =
            Binop.check_binop ~check_defined ~expected env outer p op e1 e2
          in
          let (env, te2, ty2) =
            Binop.check_binop
              ~check_defined
              ~expected
              env
              outer
              p
              (Ast_defs.Eq None)
              e1
              (Either.Second (te, ty))
          in
          let te_opt = resugar_binop te2 in
          begin
            match te_opt with
            | Some (_, _, te) -> make_result env p te ty2
            | _ -> assert false
          end)
      | None ->
        let (env, te2, ty2) =
          check_e2
            (Expr.raw_expr
               ~expected:None
               ~config:Expr.Config.{ default with check_defined }
               ~ctxt:Expr.Context.default)
            env
            e2
        in
        let (_, pos2, _) = te2 in
        let (env, te1, ty, ty_mismatch_opt) = Assign.assign p env e1 pos2 ty2 in
        let te =
          Aast.Binop
            {
              bop = Ast_defs.Eq None;
              lhs = te1;
              rhs = hole_on_ty_mismatch ~ty_mismatch_opt te2;
            }
        in
        make_result env p te ty)
    | Ast_defs.Ampamp
    | Ast_defs.Barbar ->
      let c = Ast_defs.(equal_bop bop Ampamp) in
      let (env, te1, _) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.{ default with check_defined }
          ~ctxt:Expr.Context.default
          env
          e1
      in
      let lenv = env.lenv in
      let (env, _) = Expr.condition env c te1 in
      let (env, te2, _) =
        check_e2
          (Expr.expr
             ~expected:None
             ~config:Expr.Config.{ default with check_defined }
             ~ctxt:Expr.Context.default)
          env
          e2
      in
      let env = { env with lenv } in
      make_result
        env
        p
        (Aast.Binop { bop; lhs = te1; rhs = te2 })
        (MakeType.bool (Reason.Rlogic_ret p))
    | _ ->
      let (env, te1, ty1) =
        Expr.raw_expr
          ~expected:None
          ~config:Expr.Config.{ default with check_defined }
          ~ctxt:Expr.Context.default
          env
          e1
      in
      let (env, te2, ty2) =
        check_e2
          (Expr.raw_expr
             ~expected:None
             ~config:Expr.Config.{ default with check_defined }
             ~ctxt:Expr.Context.default)
          env
          e2
      in
      let env =
        match bop with
        (* TODO: This could be less conservative: we only need to account for
         * the possibility of exception if the operator is `/` or `/=`.
         *)
        | Ast_defs.Eqeqeq
        | Ast_defs.Diff2 ->
          env
        | _ -> might_throw ~join_pos:p env
      in
      let (_, p1, _) = e1 in
      let p2 =
        match e2 with
        | Either.First (_, p, _)
        | Either.Second ((_, p, _), _) ->
          p
      in
      let (env, te3, ty) =
        Typing_arithmetic.binop p env bop p1 te1 ty1 p2 te2 ty2
      in
      (env, te3, ty)
end

and Expression_tree : sig
  val expression_tree :
    env -> pos -> (unit, unit) expression_tree -> env * Tast.expr * locl_ty
end = struct
  let expression_tree env p et =
    let {
      et_hint;
      et_splices;
      et_function_pointers;
      et_virtualized_expr;
      et_runtime_expr;
      et_dollardollar_pos;
    } =
      et
    in

    (* Slight hack to deal with |> $$ support *)
    let env =
      match et_dollardollar_pos with
      | Some dd_pos ->
        let dollardollar_var =
          Local_id.make_unscoped SN.ExpressionTrees.dollardollarTmpVar
        in
        let dd_var = Local_id.make_unscoped SN.SpecialIdents.dollardollar in
        let dd_defined = Env.is_local_present env dd_var in
        if not dd_defined then
          let () =
            Errors.add_error
              Naming_error.(
                to_user_error
                @@ Undefined
                     {
                       pos = dd_pos;
                       var_name = SN.SpecialIdents.dollardollar;
                       did_you_mean = None;
                     })
          in
          let nothing_ty = MakeType.nothing Reason.Rnone in
          Env.set_local
            ~is_defined:true
            ~bound_ty:None
            env
            dollardollar_var
            nothing_ty
            Pos.none
        else
          let dd_local = Env.get_local env dd_var in
          Typing_local_types.(
            Env.set_local
              ~is_defined:true
              ~bound_ty:None
              env
              dollardollar_var
              dd_local.ty
              dd_local.pos)
      | None -> env
    in

    (* Given the expression tree literal:

       MyVisitor`1 + ${ foo() }`

       First, type check the expressions that are spliced in, so foo() in
       this example. *)
    let (env, t_splices) = Stmt.block env et_splices in

    (* Next, typecheck the function pointer assignments *)
    let (env, _, t_function_pointers) =
      Env.with_inside_expr_tree env et_hint (fun env ->
          let (env, t_function_pointers) =
            Stmt.block env et_function_pointers
          in
          (env, (), t_function_pointers))
    in

    (* Type check the virtualized expression, which will look
       roughly like this:

       function() {
         $0splice0 = foo();
         return MyVisitor::intType()->__plus($0splice0);
       }
    *)
    let (env, t_virtualized_expr, ty_virtual) =
      Env.with_inside_expr_tree env et_hint (fun env ->
          Expr.expr
            ~expected:None
            ~config:Expr.Config.default
            ~ctxt:Expr.Context.default
            env
            et_virtualized_expr)
    in

    (* If the virtualized expression type is pessimised, we should strip off likes.
     * Do this through supportdyn and unions.
     *)
    let rec strip_all_likes env ty =
      let (env, ty) = Env.expand_type env ty in
      match get_node ty with
      | Tnewtype (name, [ty'], _) when String.equal name SN.Classes.cSupportDyn
        ->
        let (env, ty'') = strip_all_likes env ty' in
        (env, MakeType.supportdyn (get_reason ty) ty'')
      | Tunion tyl ->
        let tyl =
          List.filter tyl ~f:(fun ty -> not (Typing_defs.is_dynamic ty))
        in
        let (env, tyl) = List.map_env env tyl ~f:strip_all_likes in
        Union.union_list env (get_reason ty) tyl
      | _ -> (env, ty)
    in

    let (env, ty_virtual) = strip_all_likes env ty_virtual in

    (* Given the runtime expression:

        MyVisitor::makeTree(...)

        add the inferred type as a type parameter:

        MyVisitor::makeTree<MyVisitorInt>(...)

       and then typecheck. *)
    let (env, runtime_expr) =
      maketree_with_type_param env p et_runtime_expr ty_virtual
    in
    let (env, t_runtime_expr, ty_runtime_expr) =
      Expr.expr
        ~expected:None
        ~config:Expr.Config.default
        ~ctxt:Expr.Context.default
        env
        runtime_expr
    in

    make_result
      env
      p
      (Aast.ExpressionTree
         {
           et_hint;
           et_splices = t_splices;
           et_function_pointers = t_function_pointers;
           et_virtualized_expr = t_virtualized_expr;
           et_runtime_expr = t_runtime_expr;
           et_dollardollar_pos;
         })
      ty_runtime_expr
end

and Afield : sig
  val array_field :
    env ->
    (unit, unit) afield ->
    env
    * ((locl_ty, saved_env) afield
      * locl_phase Typing_defs_core.ty option
      * locl_ty)
end = struct
  let array_field env = function
    | AFvalue ve ->
      let (env, tve, tv) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          ve
      in
      (env, (Aast.AFvalue tve, None, tv))
    | AFkvalue (ke, ve) ->
      let (env, tke, tk) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          ke
      in
      let (env, tve, tv) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          ve
      in
      (env, (Aast.AFkvalue (tke, tve), Some tk, tv))
end

and Xhp_attribute : sig
  val xhp_check_get_attribute :
    pos -> env -> Nast.expr -> byte_string -> og_null_flavor -> env

  (**
  * Typecheck the attribute expressions - this just checks that the expressions are
  * valid, not that they match the declared type for the attribute and,
  * in case of spreads, makes sure they are XHP.
  *)
  val xhp_attribute_exprs :
    env ->
    Cls.t option ->
    (unit, unit) xhp_attribute list ->
    sid ->
    locl_ty ->
    env * (locl_ty, saved_env) xhp_attribute list
end = struct
  let xhp_check_get_attribute pos env base_expr prop null_flavour =
    (* All and only attributes start with `:` when represented as properties. *)
    if String.is_prefix prop ~prefix:":" then
      let base_lenv = env.lenv in
      let call_get_attr =
        Typing_xhp.rewrite_attribute_access_into_call pos base_expr null_flavour
      in
      let (env, _, _) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.default
          env
          call_get_attr
      in
      (* Snap the local env back to avoid manipualting refinements twice. *)
      { env with lenv = base_lenv }
    else
      env

  (**
  * Process a spread operator by computing the intersection of XHP attributes
  * between the spread expression and the XHP constructor onto which we're
  * spreading.
  *)
  let xhp_spread_attribute env c_onto valexpr sid obj =
    let (_, p, _) = valexpr in
    let (env, te, valty) =
      Expr.expr
        ~expected:None
        ~config:Expr.Config.default
        ~ctxt:Expr.Context.default
        env
        valexpr
    in
    let ((env, xhp_req_err_opt), attr_ptys) =
      match c_onto with
      | None -> ((env, None), [])
      | Some class_info ->
        Typing_xhp.get_spread_attributes env p class_info valty
    in
    Option.iter xhp_req_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    let (env, has_ty_mismatch) =
      List.fold_left
        attr_ptys
        ~f:(fun (env, has_err) attr ->
          let (env, _, err_opt) = xhp_attribute_decl_ty env sid obj attr in
          (env, has_err || Option.is_some err_opt))
        ~init:(env, false)
    in
    (* If we have a subtyping error for any attribute, the best we can do here
       is give an expected type of nothing *)
    let ty_mismatch_opt =
      if has_ty_mismatch then
        Some (valty, MakeType.nothing Reason.Rnone)
      else
        None
    in
    (* Build the typed attribute node *)
    let typed_attr =
      Aast.Xhp_spread (hole_on_ty_mismatch ~ty_mismatch_opt te)
    in
    (env, typed_attr)

  (**
  * Simple XHP attributes (attr={expr} form) are simply interpreted as a member
  * variable prefixed with a colon.
  *)
  let xhp_simple_attribute env id valexpr sid obj =
    let (_, p, _) = valexpr in
    let (env, te, valty) =
      Expr.expr
        ~expected:None
        ~config:Expr.Config.default
        ~ctxt:Expr.Context.default
        env
        valexpr
    in
    (* This converts the attribute name to a member name. *)
    let name = ":" ^ snd id in
    let attr_pty = ((fst id, name), (p, valty)) in
    let (env, decl_ty, ty_mismatch_opt) =
      xhp_attribute_decl_ty env sid obj attr_pty
    in
    let typed_attr =
      Aast.Xhp_simple
        {
          xs_name = id;
          xs_type = decl_ty;
          xs_expr = hole_on_ty_mismatch ~ty_mismatch_opt te;
        }
    in
    (env, typed_attr)

  (**
  * Typecheck the attribute expressions - this just checks that the expressions are
  * valid, not that they match the declared type for the attribute and,
  * in case of spreads, makes sure they are XHP.
  *)
  let xhp_attribute_exprs env cls_decl attrl sid obj =
    let handle_attr (env, typed_attrl) attr =
      let (env, typed_attr) =
        match attr with
        | Xhp_simple { xs_name = id; xs_expr = valexpr; _ } ->
          xhp_simple_attribute env id valexpr sid obj
        | Xhp_spread valexpr ->
          xhp_spread_attribute env cls_decl valexpr sid obj
      in
      (env, typed_attr :: typed_attrl)
    in
    let (env, typed_attrl) =
      List.fold_left ~f:handle_attr ~init:(env, []) attrl
    in
    (env, List.rev typed_attrl)
end

and As_expr : sig
  val bind_as_expr :
    env ->
    pos ->
    locl_phase Typing_defs_core.ty ->
    locl_ty ->
    (unit, unit) as_expr ->
    env * (locl_ty, saved_env) as_expr
end = struct
  let bind_as_expr env p ty1 ty2 aexpr =
    match aexpr with
    | As_v ev ->
      let (env, te, _, _) = Assign.assign p env ev p ty2 in
      (env, Aast.As_v te)
    | Await_as_v (p, ev) ->
      let (env, te, _, _) = Assign.assign p env ev p ty2 in
      (env, Aast.Await_as_v (p, te))
    | As_kv ((_, p, Lvar ((_, k) as id)), ev) ->
      let env = set_valid_rvalue ~is_defined:true p env k None ty1 in
      let (env, te, _, _) = Assign.assign p env ev p ty2 in
      let (env, tk) =
        TUtils.make_simplify_typed_expr env p ty1 (Aast.Lvar id)
      in
      (env, Aast.As_kv (tk, te))
    | As_kv ((_, p, (Lplaceholder _ as k)), ev) ->
      let (env, te, _, _) = Assign.assign p env ev p ty2 in
      let (env, tk) = TUtils.make_simplify_typed_expr env p ty1 k in
      (env, Aast.As_kv (tk, te))
    | Await_as_kv (p, (_, p1, Lvar ((_, k) as id)), ev) ->
      let env = set_valid_rvalue ~is_defined:true p env k None ty1 in
      let (env, te, _, _) = Assign.assign p env ev p ty2 in
      let (env, tk) =
        TUtils.make_simplify_typed_expr env p1 ty1 (Aast.Lvar id)
      in
      (env, Aast.Await_as_kv (p, tk, te))
    | Await_as_kv (p, (_, p1, (Lplaceholder _ as k)), ev) ->
      let (env, te, _, _) = Assign.assign p env ev p ty2 in
      let (env, tk) = TUtils.make_simplify_typed_expr env p1 ty1 k in
      (env, Aast.Await_as_kv (p, tk, te))
    | _ ->
      (* TODO Probably impossible, should check that *)
      assert false
end

and Using_stmt : sig
  val check_using_clause :
    env ->
    bool ->
    (unit * pos * Nast.expr_) list ->
    env * Tast.expr list * local_id list
end = struct
  (* Ensure that `ty` is a subtype of IDisposable (for `using`) or
   * IAsyncDisposable (for `await using`)
   *)
  let has_dispose_method env has_await p e ty =
    let meth =
      if has_await then
        SN.Members.__disposeAsync
      else
        SN.Members.__dispose
    in
    let (_, obj_pos, _) = e in
    let ((env, ty_err_opt), (tfty, _tal)) =
      TOG.obj_get
        ~obj_pos
        ~is_method:true
        ~inst_meth:false
        ~meth_caller:false
        ~nullsafe:None
        ~coerce_from_ty:None
        ~explicit_targs:[]
        ~class_id:(CIexpr e)
        ~member_id:(p, meth)
        ~on_error:(Typing_error.Callback.using_error p ~has_await)
        env
        ty
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    let (env, (_tel, _typed_unpack_element, _ty, _should_forget_fakes)) =
      Expr.call
        ~expected:None
        ~expr_pos:p
        ~recv_pos:obj_pos
        ~in_await:None
        env
        tfty
        []
        None
    in
    env

  (* Check an individual component in the expression `e` in the
   * `using (e) { ... }` statement.
   * This consists of either
   *   a simple assignment `$x = e`, in which `$x` is the using variable, or
   *   an arbitrary expression `e`, in which case a temporary is the using
   *      variable, inaccessible in the source.
   * Return the typed expression and its type, and any variables that must
   * be designated as "using variables" for avoiding escapes.
   *)
  let check_using_expr has_await env ((_, pos, content) as using_clause) =
    match content with
    (* Simple assignment to local of form `$lvar = e` *)
    | Binop { bop = Ast_defs.Eq None; lhs = (_, lvar_pos, Lvar lvar); rhs = e }
      ->
      let (env, te, ty) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.{ default with is_using_clause = true }
          env
          e
      in
      let env = has_dispose_method env has_await pos e ty in
      let env =
        set_valid_rvalue
          ~is_using_clause:true
          ~is_defined:true
          (fst lvar)
          env
          (snd lvar)
          None
          ty
      in
      let (env, inner_tast) =
        TUtils.make_simplify_typed_expr env lvar_pos ty (Aast.Lvar lvar)
      in
      let (env, tast) =
        TUtils.make_simplify_typed_expr
          env
          pos
          ty
          (Aast.Binop { bop = Ast_defs.Eq None; lhs = inner_tast; rhs = te })
      in
      (env, (tast, [snd lvar]))
    (* Arbitrary expression. This will be assigned to a temporary *)
    | _ ->
      let (env, typed_using_clause, ty) =
        Expr.expr
          ~expected:None
          ~config:Expr.Config.default
          ~ctxt:Expr.Context.{ default with is_using_clause = true }
          env
          using_clause
      in
      let env = has_dispose_method env has_await pos using_clause ty in
      (env, (typed_using_clause, []))

  let check_using_clause env has_await using_clauses =
    let (env, pairs) =
      List.map_env env using_clauses ~f:(check_using_expr has_await)
    in
    let (typed_using_clauses, vars) = List.unzip pairs in
    (env, typed_using_clauses, List.concat vars)
end

and Assign : sig
  val assign :
    pos ->
    env ->
    Nast.expr ->
    pos ->
    locl_ty ->
    env
    * Tast.expr
    * locl_ty
    * (locl_ty * locl_phase Typing_defs_core.ty) option

  val assign_ :
    pos ->
    Reason.ureason ->
    env ->
    Nast.expr ->
    pos ->
    locl_ty ->
    env * Tast.expr * locl_ty
end = struct
  let assign_simple pos ur env e1 ty2 =
    let (env, te1, ty1) = Expr.lvalue env e1 in
    let (env, ty_err_opt) =
      Typing_coercion.coerce_type
        pos
        ur
        env
        ty2
        (MakeType.unenforced ty1)
        Typing_error.Callback.unify_error
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    let ty_mismatch = mk_ty_mismatch_opt ty2 ty1 ty_err_opt in
    (env, te1, ty2, ty_mismatch)

  let rec assign_with_subtype_err_ p ur env (e1 : Nast.expr) pos2 ty2 =
    match e1 with
    | (_, _, Hole (e, _, _, _)) -> assign_with_subtype_err_ p ur env e pos2 ty2
    | _ ->
      let env =
        match e1 with
        | (_, _, Lvar (_, x)) ->
          Env.forget_prefixed_members env x Reason.(Blame (p, BSassignment))
        (* If we ever extend fake members from $x->a to more complicated lvalues
           such as $x->a->b, we would need to call forget_prefixed_members on
           other lvalues as well. *)
        | (_, _, Obj_get (_, (_, _, Id (_, property)), _, _)) ->
          Env.forget_suffixed_members
            env
            property
            Reason.(Blame (p, BSassignment))
        | _ -> env
      in
      (match e1 with
      | (_, _, Lvar ((_, x) as id)) ->
        let env = set_valid_rvalue ~is_defined:true p env x None ty2 in
        let (_, p1, _) = e1 in
        let (env, te, ty) = make_result env p1 (Aast.Lvar id) ty2 in
        (env, te, ty, None)
      | (_, _, Lplaceholder id) ->
        let placeholder_ty = MakeType.void (Reason.Rplaceholder p) in
        let (_, p1, _) = e1 in
        let (env, te, ty) =
          make_result env p1 (Aast.Lplaceholder id) placeholder_ty
        in
        (env, te, ty, None)
      | (_, _, List el) ->
        (* Generate fresh types for each lhs list element, then subtype against
           rhs type. If original type was generated from an error, propagate this to
           avoid cascading errors. *)
        let is_error_type = TUtils.is_tyvar_error env ty2 in
        let env = Env.open_tyvars env p in
        let (env, tyl) =
          List.map_env env el ~f:(fun env (_, p, _e) ->
              if is_error_type then
                Env.fresh_type_error env p
              else
                Env.fresh_type env p)
        in
        let (_, p1, _) = e1 in
        let destructure_ty =
          MakeType.list_destructure (Reason.Rdestructure p1) tyl
        in
        let env = Env.set_tyvar_variance_i env destructure_ty in
        let lty2 = LoclType ty2 in
        let assign_accumulate (env, tel, errs) (lvalue : Nast.expr) ty2 =
          let (env, te, _, err_opt) = assign p env lvalue pos2 ty2 in
          (env, te :: tel, err_opt :: errs)
        in
        let type_list_elem env =
          let (env, reversed_tel, rev_subtype_errs) =
            List.fold2_exn el tyl ~init:(env, [], []) ~f:assign_accumulate
          in
          let (_, p1, _) = e1 in
          let (env, te, ty) =
            make_result env p1 (Aast.List (List.rev reversed_tel)) ty2
          in
          (env, te, ty, List.rev rev_subtype_errs)
        in

        (* Here we attempt to unify the type of the rhs we assigning with
           a number of fresh tyvars corresponding to the arity of the lhs `list`

           if we have a failure in subtyping the fresh tyvars in `destructure_ty`,
           we have a rhs which cannot be destructured to the variables on the lhs;
           e.g. `list($x,$y) = 2`  or `list($x,$y) = tuple (1,2,3)`

           in the error case, we add a `Hole` with expected type `nothing` since
           there is no type we can suggest was expected

           in the ok case were the destrucutring succeeded, the fresh vars
           now have types so we can subtype each element, accumulate the errors
           and pack back into the rhs structure as our expected type *)
        let (env, ty_err1) =
          Type.sub_type_i
            p
            ur
            env
            lty2
            destructure_ty
            Typing_error.Callback.unify_error
        in
        let (env, ty_err2) = Typing_solver.close_tyvars_and_solve env in
        let ty_err_opt = Option.merge ty_err1 ty_err2 ~f:Typing_error.both in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        (match ty_err_opt with
        | None ->
          let (env, te, ty, subty_errs) = type_list_elem env in
          let err_opt =
            if List.for_all subty_errs ~f:Option.is_none then
              None
            else
              Some (ty2, pack_errs pos2 ty2 (subty_errs, None))
          in
          (env, te, ty, err_opt)
        | Some _ ->
          let (env, te, ty, _) = type_list_elem env in
          let nothing =
            MakeType.nothing @@ Reason.Rsolve_fail (Pos_or_decl.of_raw_pos pos2)
          in
          (env, te, ty, Some (ty2, nothing)))
      | ( _,
          pobj,
          Obj_get
            ( obj,
              (_, pm, Id ((_, member_name) as m)),
              nullflavor,
              prop_or_method ) ) ->
        let lenv = env.lenv in
        let nullsafe =
          match nullflavor with
          | OG_nullthrows -> None
          | OG_nullsafe -> Some (Reason.Rnullsafe_op pobj)
        in
        let (env, tobj, obj_ty) =
          Expr.expr
            ~expected:None
            ~config:Expr.Config.{ default with accept_using_var = true }
            ~ctxt:Expr.Context.default
            env
            obj
        in
        let env = might_throw ~join_pos:p env in
        let (_, p1, _) = obj in
        let ( (env, ty_err_opt),
              (declared_ty, _tal),
              lval_ty_mismatch_opt,
              rval_ty_mismatch_opt ) =
          TOG.obj_get_with_mismatches
            ~obj_pos:p1
            ~is_method:false
            ~nullsafe
            ~inst_meth:false
            ~meth_caller:false
            ~coerce_from_ty:(Some (p, ur, ty2))
            ~explicit_targs:[]
            ~class_id:(CIexpr e1)
            ~member_id:m
            ~on_error:Typing_error.Callback.unify_error
            env
            obj_ty
        in
        Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
        let (env, inner_te) =
          TUtils.make_simplify_typed_expr env pm declared_ty (Aast.Id m)
        in
        let (env, te1) =
          TUtils.make_simplify_typed_expr
            env
            pobj
            declared_ty
            (Aast.Obj_get
               ( hole_on_ty_mismatch ~ty_mismatch_opt:lval_ty_mismatch_opt tobj,
                 inner_te,
                 nullflavor,
                 prop_or_method ))
        in
        let env = { env with lenv } in
        begin
          match obj with
          | (_, _, This)
          | (_, _, Lvar _) ->
            let (env, local) = Env.FakeMembers.make env obj member_name p in
            let (env, refined_ty) =
              Inter.intersect env ~r:(Reason.Rwitness p) declared_ty ty2
            in
            let env =
              set_valid_rvalue ~is_defined:true p env local None refined_ty
            in
            (env, te1, ty2, rval_ty_mismatch_opt)
          | _ -> (env, te1, ty2, rval_ty_mismatch_opt)
        end
      | (_, _, Class_get (((_, _, x) as cid), CGstring (pos_member, y), _)) ->
        let lenv = env.lenv in
        let no_fakes = LEnv.env_with_empty_fakes env in
        let (env, te1, _) = Expr.lvalue no_fakes e1 in
        let env = { env with lenv } in
        let (env, ety2) = Env.expand_type env ty2 in
        (* This defers the coercion check to class_get, which looks up the appropriate target type *)
        let (env, _tal, _, cty) = Class_id.class_expr env [] cid in
        let env = might_throw ~join_pos:p env in
        let (env, (declared_ty, _), rval_err_opt) =
          Class_get_expr.class_get_err
            ~is_method:false
            ~is_const:false
            ~transform_fty:None
            ~coerce_from_ty:(Some (p, ur, ety2))
            env
            cty
            (pos_member, y)
            cid
        in
        let (env, local) = Env.FakeMembers.make_static env x y p in
        let (env, refined_ty) =
          Inter.intersect env ~r:(Reason.Rwitness p) declared_ty ty2
        in
        let env =
          set_valid_rvalue ~is_defined:true p env local None refined_ty
        in
        (env, te1, ty2, rval_err_opt)
      | (_, _, Obj_get _)
      | (_, _, Class_get _) ->
        let lenv = env.lenv in
        let no_fakes = LEnv.env_with_empty_fakes env in
        let (env, te1, real_type) = Expr.lvalue no_fakes e1 in
        let (env, exp_real_type) = Env.expand_type env real_type in
        let env = { env with lenv } in
        let (env, ty_err_opt) =
          Typing_coercion.coerce_type
            p
            ur
            env
            ty2
            (MakeType.unenforced exp_real_type)
            Typing_error.Callback.unify_error
        in
        Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
        let ty_mismatch = mk_ty_mismatch_opt ty2 exp_real_type ty_err_opt in
        (env, te1, ty2, ty_mismatch)
      | (_, pos, Array_get (e1, None)) ->
        let parent_lenv = env.lenv in
        let (env, te1, ty1) =
          Expr.update_array_type
            pos
            env
            e1
            Valkind.Lvalue
            ~lhs_of_null_coalesce:false
        in
        let (_, p1, _) = e1 in
        let (env, (ty1', arr_ty_mismatch_opt, val_ty_mismatch_opt)) =
          Typing_array_access.assign_array_append
            ~array_pos:p1
            ~expr_pos:p
            ur
            env
            ty1
            ty2
        in
        let (ty1_is_hack_collection, ty_err_opt) = is_hack_collection env ty1 in
        let (env, te1) =
          if ty1_is_hack_collection then
            (env, hole_on_ty_mismatch ~ty_mismatch_opt:arr_ty_mismatch_opt te1)
          else
            let env = { env with lenv = parent_lenv } in
            let env = might_throw ~join_pos:p env in
            let (env, te1, ty, _) =
              assign_with_subtype_err_ p ur env e1 p1 ty1'
            in
            (* Update the actual type to that after assignment *)
            let arr_ty_mismatch_opt =
              Option.map arr_ty_mismatch_opt ~f:(fun (_, ty_expect) ->
                  (ty, ty_expect))
            in
            (env, hole_on_ty_mismatch ~ty_mismatch_opt:arr_ty_mismatch_opt te1)
        in
        let (env, te, ty) =
          make_result env pos (Aast.Array_get (te1, None)) ty2
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        (env, te, ty, val_ty_mismatch_opt)
      | (_, pos, Array_get (e1, Some e)) ->
        let (env, te, ty) =
          Expr.expr
            ~expected:None
            ~config:Expr.Config.default
            ~ctxt:Expr.Context.default
            env
            e
        in
        let parent_lenv = env.lenv in
        let (env, te1, ty1) =
          Expr.update_array_type
            pos
            env
            e1
            Valkind.Lvalue
            ~lhs_of_null_coalesce:false
        in
        let env = might_throw ~join_pos:p env in
        let (_, p1, _) = e1 in
        let ( env,
              ( ty1',
                arr_ty_mismatch_opt,
                key_ty_mismatch_opt,
                val_ty_mismatch_opt ) ) =
          Typing_array_access.assign_array_get
            ~array_pos:p1
            ~expr_pos:p
            ur
            env
            ty1
            e
            ty
            ty2
        in
        let (ty1_is_hack_collection, ty_err_opt) = is_hack_collection env ty1 in
        let (env, te1) =
          if ty1_is_hack_collection then
            (env, hole_on_ty_mismatch ~ty_mismatch_opt:arr_ty_mismatch_opt te1)
          else
            let env = { env with lenv = parent_lenv } in
            let env = might_throw ~join_pos:p env in
            let (env, te1, ty, _) =
              assign_with_subtype_err_ p ur env e1 p1 ty1'
            in
            (* Update the actual type to that after assignment *)
            let arr_ty_mismatch_opt =
              Option.map arr_ty_mismatch_opt ~f:(fun (_, ty_expect) ->
                  (ty, ty_expect))
            in
            (env, hole_on_ty_mismatch ~ty_mismatch_opt:arr_ty_mismatch_opt te1)
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        ( env,
          ( ty2,
            pos,
            Aast.Array_get
              ( te1,
                Some
                  (hole_on_ty_mismatch ~ty_mismatch_opt:key_ty_mismatch_opt te)
              ) ),
          ty2,
          val_ty_mismatch_opt )
      | (_, pos, ReadonlyExpr e) ->
        let (env, te1, ty, err) =
          assign_with_subtype_err_ pos ur env e pos2 ty2
        in
        (env, (ty, pos, Aast.ReadonlyExpr te1), ty, err)
      | _ -> assign_simple p ur env e1 ty2)

  (* Deal with assignment of a value of type ty2 to lvalue e1 *)
  and assign p env e1 pos2 ty2 =
    assign_with_subtype_err_ p Reason.URassign env e1 pos2 ty2

  let assign_ p ur env e1 pos2 ty2 =
    let (env, te, ty, _err) = assign_with_subtype_err_ p ur env e1 pos2 ty2 in
    (env, te, ty)
end

and Class_get_expr : sig
  val class_get :
    is_method:bool ->
    is_const:bool ->
    transform_fty:
      (decl_phase Typing_defs_core.ty fun_type ->
      decl_phase Typing_defs_core.ty fun_type)
      option ->
    coerce_from_ty:
      (pos * Reason.ureason * locl_phase Typing_defs_core.ty) option ->
    ?is_attribute_param:bool ->
    ?explicit_targs:Nast.targ list ->
    ?incl_tc:bool ->
    ?is_function_pointer:bool ->
    env ->
    locl_phase Typing_defs_core.ty ->
    pos * byte_string ->
    Nast.class_id ->
    env * (locl_ty * Tast.targ list)

  val class_get_err :
    is_method:bool ->
    is_const:bool ->
    transform_fty:(decl_fun_type -> decl_fun_type) option ->
    coerce_from_ty:(pos * Reason.ureason * locl_ty) option ->
    ?explicit_targs:Nast.targ list ->
    ?incl_tc:bool ->
    ?is_function_pointer:bool ->
    env ->
    locl_ty ->
    pos * byte_string ->
    Nast.class_id ->
    env
    * (locl_ty * Tast.targ list)
    * (locl_ty * locl_phase Typing_defs_core.ty) option
end = struct
  let rec class_get_res
      ~is_method
      ~is_const
      ~transform_fty
      ~coerce_from_ty
      ?(is_attribute_param = false)
      ?(explicit_targs = [])
      ?(incl_tc = false)
      ?(is_function_pointer = false)
      env
      cty
      (p, mid)
      cid =
    let (env, this_ty) =
      if is_method then
        Class_id.this_for_method env cid cty
      else
        (env, cty)
    in
    class_get_inner
      ~is_method
      ~is_const
      ~transform_fty
      ~this_ty
      ~explicit_targs
      ~incl_tc
      ~coerce_from_ty
      ~is_function_pointer
      ~is_attribute_param
      env
      cid
      cty
      (p, mid)

  and class_get_err
      ~is_method
      ~is_const
      ~transform_fty
      ~coerce_from_ty
      ?explicit_targs
      ?incl_tc
      ?is_function_pointer
      env
      cty
      (p, mid)
      cid =
    let (env, tys, rval_res_opt) =
      class_get_res
        ~is_method
        ~is_const
        ~transform_fty
        ~coerce_from_ty
        ?explicit_targs
        ?incl_tc
        ?is_function_pointer
        env
        cty
        (p, mid)
        cid
    in
    let rval_err_opt = Option.bind ~f:Result.error rval_res_opt in
    (env, tys, rval_err_opt)

  and class_get
      ~is_method
      ~is_const
      ~transform_fty
      ~coerce_from_ty
      ?(is_attribute_param = false)
      ?explicit_targs
      ?incl_tc
      ?is_function_pointer
      env
      cty
      (p, mid)
      cid =
    let (env, tys, _) =
      class_get_res
        ~is_method
        ~is_const
        ~transform_fty
        ~coerce_from_ty
        ~is_attribute_param
        ?explicit_targs
        ?incl_tc
        ?is_function_pointer
        env
        cty
        (p, mid)
        cid
    in
    (env, tys)

  and class_get_inner
      ~is_method
      ~is_const
      ~this_ty
      ~coerce_from_ty
      ~transform_fty
      ?(explicit_targs = [])
      ?(incl_tc = false)
      ?(is_function_pointer = false)
      ?(is_attribute_param = false)
      env
      ((_, _cid_pos, cid_) as cid)
      cty
      (p, mid) =
    let (env, cty) = Env.expand_type env cty in
    let dflt_rval_err =
      Option.map ~f:(fun (_, _, ty) -> Ok ty) coerce_from_ty
    in
    match deref cty with
    | (_, (Tdynamic | Tany _)) -> (env, (cty, []), dflt_rval_err)
    | (_, Tunion tyl) ->
      let (env, pairs, rval_err_opts) =
        List.fold_left
          tyl
          ~init:(env, [], [])
          ~f:(fun (env, pairs, rval_err_opts) ty ->
            let (env, pair, rval_err_opt) =
              class_get_res
                ~is_method
                ~is_const
                ~transform_fty
                ~explicit_targs
                ~incl_tc
                ~coerce_from_ty
                ~is_function_pointer
                env
                ty
                (p, mid)
                cid
            in
            (env, pair :: pairs, rval_err_opt :: rval_err_opts))
      in

      let rval_err = Option.(map ~f:union_coercion_errs @@ all rval_err_opts) in
      let (env, ty) =
        Union.union_list env (get_reason cty) (List.map ~f:fst pairs)
      in
      (* Pick up the maximal number of type arguments to put in the TAST, so that
         TAST checks such as the reified checks can check the type arguments
         against the spec. *)
      let tal =
        List.map ~f:snd pairs
        |> List.max_elt ~compare:(fun a b ->
               Int.compare (List.length a) (List.length b))
        |> Option.value ~default:[]
      in
      (env, (ty, tal), rval_err)
    | (_, Tintersection tyl) ->
      let (env, pairs, rval_err_opts) =
        TUtils.run_on_intersection_res env tyl ~f:(fun env ty ->
            class_get_inner
              ~is_method
              ~is_const
              ~transform_fty
              ~this_ty
              ~explicit_targs
              ~incl_tc
              ~coerce_from_ty
              ~is_function_pointer
              ~is_attribute_param
              env
              cid
              ty
              (p, mid))
      in
      let rval_err =
        Option.(map ~f:intersect_coercion_errs @@ all rval_err_opts)
      in
      let (env, ty) =
        Inter.intersect_list env (get_reason cty) (List.map ~f:fst pairs)
      in
      (env, (ty, []), rval_err)
    | (_, Tnewtype (_, _, ty))
    | (_, Tdependent (_, ty)) ->
      class_get_inner
        ~is_method
        ~is_const
        ~transform_fty
        ~this_ty
        ~explicit_targs
        ~incl_tc
        ~coerce_from_ty
        ~is_function_pointer
        ~is_attribute_param
        env
        cid
        ty
        (p, mid)
    | (r, Tgeneric _) ->
      let (env, tyl) =
        TUtils.get_concrete_supertypes ~abstract_enum:true env cty
      in
      let (env, has_no_bound) =
        TUtils.no_upper_bound ~include_sd_mixed:true env tyl
      in
      if has_no_bound then begin
        let (env, ty) = Env.fresh_type_error env p in
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Non_class_member
                 {
                   elt =
                     (if is_method then
                       `meth
                     else
                       `prop);
                   member_name = mid;
                   pos = p;
                   ty_name = lazy (Typing_print.error env cty);
                   decl_pos = get_pos cty;
                 });

        (env, (ty, []), dflt_rval_err)
      end else
        let (env, ty) = Inter.intersect_list env r tyl in
        class_get_inner
          ~is_method
          ~is_const
          ~transform_fty
          ~this_ty
          ~explicit_targs
          ~incl_tc
          ~coerce_from_ty
          ~is_function_pointer
          ~is_attribute_param
          env
          cid
          ty
          (p, mid)
    | (_, Tclass ((_, c), _, paraml)) ->
      let class_ = Env.get_class env c in
      (match class_ with
      | None ->
        let ty = MakeType.nothing (Reason.Rmissing_class p) in
        (env, (ty, []), dflt_rval_err)
      | Some class_ ->
        (* TODO akenn: Should we move this to the class_get original call? *)
        let (env, this_ty) = ExprDepTy.make env ~cid:cid_ this_ty in
        (* We need to instantiate generic parameters in the method signature *)
        let ety_env =
          {
            empty_expand_env with
            this_ty;
            substs = TUtils.make_locl_subst_for_class_tparams class_ paraml;
          }
        in
        let get_smember_from_constraints env class_info =
          (* Extract the upper bounds on this from where and require class constraints. *)
          let upper_bounds_from_where_constraints =
            Cls.upper_bounds_on_this_from_constraints class_info
          in
          let upper_bounds_from_require_class_constraints =
            List.map (Cls.all_ancestor_req_class_requirements class_info) ~f:snd
          in
          let (env, ty_err_opt, upper_bounds) =
            let ((env, ty_err_opt), upper_bounds) =
              List.map_env_ty_err_opt
                env
                (upper_bounds_from_where_constraints
                @ upper_bounds_from_require_class_constraints)
                ~f:(fun env up -> Phase.localize ~ety_env env up)
                ~combine_ty_errs:Typing_error.multiple_opt
            in
            (* If class C uses a trait that require class C, then decls for class C
             * include the require class C constraint.  This must be filtered out to avoid
             * that resolving a static element on class C enters into an infinite recursion.
             * Similarly, upper bounds on this equivalent via aliases to class C
             * introduced via class-level where clauses, must be filtered out; this is done
             * by localizing the bounds before comparing them with the current class name.
             *)
            let upper_bounds =
              List.filter
                ~f:(fun ty ->
                  match get_node ty with
                  | Tclass ((_, cn), _, _) ->
                    String.( <> ) cn (Cls.name class_info)
                  | _ -> true)
                upper_bounds
            in
            (env, ty_err_opt, upper_bounds)
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          if List.is_empty upper_bounds then begin
            (* there are no upper bounds, raise an error *)
            Typing_error_utils.add_typing_error ~env
            @@ TOG.smember_not_found
                 p
                 ~is_const
                 ~is_method
                 ~is_function_pointer
                 class_info
                 mid
                 Typing_error.Callback.unify_error;
            let (env, ty) = Env.fresh_type_error env p in
            (env, (ty, []), dflt_rval_err)
          end else
            (* since there are upper bounds on this, repeat the search on their intersection *)
            let (env, inter_ty) =
              Inter.intersect_list env (Reason.Rwitness p) upper_bounds
            in
            class_get_inner
              ~is_method
              ~is_const
              ~transform_fty
              ~this_ty
              ~explicit_targs
              ~incl_tc
              ~coerce_from_ty
              ~is_function_pointer
              ~is_attribute_param
              env
              cid
              inter_ty
              (p, mid)
        in
        let try_get_smember_from_constraints env class_info =
          Errors.try_with_error
            (fun () -> get_smember_from_constraints env class_info)
            (fun () ->
              Typing_error_utils.add_typing_error ~env
              @@ TOG.smember_not_found
                   p
                   ~is_const
                   ~is_method
                   ~is_function_pointer
                   class_info
                   mid
                   Typing_error.Callback.unify_error;
              let (env, ty) = Env.fresh_type_error env p in
              (env, (ty, []), dflt_rval_err))
        in
        if is_const then (
          let const =
            if incl_tc then
              Env.get_const env class_ mid
            else
              match Env.get_typeconst env class_ mid with
              | Some _ ->
                Typing_error_utils.add_typing_error
                  ~env
                  Typing_error.(
                    primary @@ Primary.Illegal_typeconst_direct_access p);
                None
              | None -> Env.get_const env class_ mid
          in
          match const with
          | None -> try_get_smember_from_constraints env class_
          | Some { cc_type; cc_abstract; cc_pos; _ } ->
            let ((env, ty_err_opt), cc_locl_type) =
              Phase.localize ~ety_env env cc_type
            in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            (match cc_abstract with
            | CCAbstract _ ->
              (match cid_ with
              | CIstatic
              | CIexpr _ ->
                ()
              | _ ->
                let cc_name = Cls.name class_ ^ "::" ^ mid in
                let err =
                  Typing_error.(
                    primary
                    @@ Primary.Abstract_const_usage
                         { pos = p; decl_pos = cc_pos; name = cc_name })
                in
                Typing_error_utils.add_typing_error ~env err)
            | CCConcrete -> ());
            (env, (cc_locl_type, []), dflt_rval_err)
        ) else
          let static_member_opt =
            Env.get_static_member is_method env class_ mid
          in
          (match static_member_opt with
          | None ->
            (* Before raising an error, check if the classish has upper bounds on this
             * (via class-level where clauses or require class constraints); if yes, repeat
             * the search on all the upper bounds, if not raise an error.
             *)
            try_get_smember_from_constraints env class_
          | Some
              ({
                 ce_visibility = vis;
                 ce_type = (lazy member_decl_ty);
                 ce_deprecated;
                 _;
               } as ce) ->
            let def_pos = get_pos member_decl_ty in
            (* Don't need to check visibilty on class constants in an attribute *)
            if not is_attribute_param then
              Option.iter
                ~f:(Typing_error_utils.add_typing_error ~env)
                (TVis.check_class_access
                   ~is_method
                   ~use_pos:p
                   ~def_pos
                   env
                   (vis, get_ce_lsb ce)
                   cid_
                   class_);
            Option.iter
              ~f:(Typing_error_utils.add_typing_error ~env)
              (TVis.check_deprecated ~use_pos:p ~def_pos env ce_deprecated);
            check_class_get
              env
              p
              def_pos
              c
              mid
              ce
              cid
              is_function_pointer
              is_method;
            let (env, member_ty, et_enforced, tal) =
              match deref member_decl_ty with
              (* We special case Tfun here to allow passing in explicit tparams to localize_ft. *)
              | (r, Tfun ft) when is_method ->
                let ft =
                  match transform_fty with
                  | None -> ft
                  | Some f -> f ft
                in
                Option.iter
                  ~f:(Typing_error_utils.add_typing_error ~env)
                  (TVis.check_cross_package
                     ~use_pos:p
                     ~def_pos
                     env
                     ft.ft_cross_package);

                let ((env, ty_err_opt1), explicit_targs) =
                  Phase.localize_targs
                    ~check_well_kinded:true
                    ~is_method:true
                    ~def_pos
                    ~use_pos:p
                    ~use_name:(strip_ns mid)
                    env
                    ft.ft_tparams
                    (List.map ~f:snd explicit_targs)
                in
                Option.iter
                  ~f:(Typing_error_utils.add_typing_error ~env)
                  ty_err_opt1;
                let ft =
                  Typing_enforceability.compute_enforced_and_pessimize_fun_type
                    ~this_class:(Some class_)
                    env
                    ft
                in

                let ((env, ty_err_opt2), ft) =
                  Phase.(
                    localize_ft
                      ~instantiation:
                        { use_name = strip_ns mid; use_pos = p; explicit_targs }
                      ~ety_env
                      ~def_pos
                      env
                      ft)
                in
                Option.iter
                  ~f:(Typing_error_utils.add_typing_error ~env)
                  ty_err_opt2;
                let fty =
                  Typing_dynamic.maybe_wrap_with_supportdyn
                    ~should_wrap:
                      (get_ce_support_dynamic_type ce
                      && TCO.enable_sound_dynamic env.genv.tcopt)
                    (Reason.localize r)
                    ft
                in
                (env, fty, Unenforced, explicit_targs)
              (* unused *)
              | _ ->
                let { et_type; et_enforced } =
                  Typing_enforceability.compute_enforced_and_pessimize_ty
                    ~this_class:(Some class_)
                    env
                    member_decl_ty
                in
                let ((env, ty_err_opt), member_ty) =
                  Phase.localize ~ety_env env et_type
                in
                Option.iter
                  ~f:(Typing_error_utils.add_typing_error ~env)
                  ty_err_opt;
                (* TODO(T52753871) make function just return possibly_enforced_ty
                 * after considering intersection case *)
                (env, member_ty, et_enforced, [])
            in
            let (env, member_ty) =
              if Cls.has_upper_bounds_on_this_from_constraints class_ then
                let ((env, (member_ty', _), _), succeed) =
                  Errors.try_
                    (fun () -> (get_smember_from_constraints env class_, true))
                    (fun _ ->
                      (* No eligible functions found in constraints *)
                      ( (env, (MakeType.mixed Reason.Rnone, []), dflt_rval_err),
                        false ))
                in
                if succeed then
                  Inter.intersect
                    env
                    ~r:(Reason.Rwitness p)
                    member_ty
                    member_ty'
                else
                  (env, member_ty)
              else
                (env, member_ty)
            in
            let ety = { et_type = member_ty; et_enforced } in
            let (env, rval_err) =
              match coerce_from_ty with
              | None -> (env, None)
              | Some (p, ur, ty) ->
                let (env, ty_err_opt) =
                  Typing_coercion.coerce_type
                    p
                    ur
                    env
                    ty
                    (TUtils.make_like_if_enforced env ety)
                    Typing_error.Callback.unify_error
                in
                Option.iter
                  ty_err_opt
                  ~f:(Typing_error_utils.add_typing_error ~env);
                let ty_mismatch = mk_ty_mismatch_res ty member_ty ty_err_opt in
                (env, Some ty_mismatch)
            in
            (env, (member_ty, tal), rval_err)))
    | (_, Tunapplied_alias _) ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | ( _,
        ( Tvar _ | Tnonnull | Tvec_or_dict _ | Toption _ | Tprim _ | Tfun _
        | Ttuple _ | Tshape _ | Taccess _ | Tneg _ ) ) ->
      if not (TUtils.is_tyvar_error env cty) then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Non_class_member
                 {
                   elt =
                     (if is_method then
                       `meth
                     else
                       `prop);
                   member_name = mid;
                   pos = p;
                   ty_name = lazy (Typing_print.error env cty);
                   decl_pos = get_pos cty;
                 });
      let (env, ty) = Env.fresh_type_error env p in
      (env, (ty, []), dflt_rval_err)
end

and File_attribute : sig
  val file_attributes :
    Typing_env_types.env ->
    Nast.file_attribute list ->
    Typing_env_types.env * Tast.file_attribute list
end = struct
  let file_attributes env file_attrs =
    (* Disable checking of error positions, as file attributes have spans that
     * aren't subspans of the class or function into which they are copied *)
    Errors.run_with_span Pos.none @@ fun () ->
    List.map_env env file_attrs ~f:(fun env fa ->
        let (env, user_attributes) =
          User_attribute.attributes_check_def
            env
            SN.AttributeKinds.file
            fa.fa_user_attributes
        in
        let env = set_tcopt_unstable_features env fa in
        ( env,
          {
            Aast.fa_user_attributes = user_attributes;
            Aast.fa_namespace = fa.fa_namespace;
          } ))
end

and Tparam : sig
  val type_param :
    Typing_env_types.env -> Nast.tparam -> Typing_env_types.env * Tast.tparam
end = struct
  let rec type_param env (t : Nast.tparam) =
    let (env, user_attributes) =
      User_attribute.attributes_check_def
        env
        SN.AttributeKinds.typeparam
        t.tp_user_attributes
    in
    let (env, tp_parameters) = List.map_env env t.tp_parameters ~f:type_param in
    ( env,
      {
        Aast.tp_variance = t.tp_variance;
        Aast.tp_name = t.tp_name;
        Aast.tp_parameters;
        Aast.tp_constraints = t.tp_constraints;
        Aast.tp_reified = reify_kind t.tp_reified;
        Aast.tp_user_attributes = user_attributes;
      } )
end

(* External API *)

let bind_params env ?can_read_globals ~no_auto_likes ctxts_opt tys params =
  Lambda.bind_params env ?can_read_globals ~no_auto_likes ctxts_opt tys params

let fun_ ?abstract ?native ?disable env return_info pos func_body fun_kind =
  Lambda.fun_ ?abstract ?native ?disable env return_info pos func_body fun_kind

let attributes_check_def env name user_attrs =
  User_attribute.attributes_check_def env name user_attrs

let call
    ~expected
    ?nullsafe
    ?in_await
    ?dynamic_func
    ~expr_pos
    ~recv_pos
    env
    fty
    params
    unpacked_element =
  Expr.call
    ~expected
    ?nullsafe
    ~in_await
    ?dynamic_func
    ~expr_pos
    ~recv_pos
    env
    fty
    params
    unpacked_element

let check_function_dynamically_callable
    ~this_class env f_name f params_decl_ty ret_locl_ty =
  Lambda.check_function_dynamically_callable
    ~this_class
    env
    f_name
    f
    params_decl_ty
    ret_locl_ty

let type_param env tparam = Tparam.type_param env tparam

let file_attributes env file_attrs =
  File_attribute.file_attributes env file_attrs

let expr ?expected env e =
  Env.with_origin2 env Decl_counters.Body (fun env ->
      Expr.expr
        ~expected
        ~config:Expr.Config.default
        ~ctxt:Expr.Context.default
        env
        e)

let expr_with_pure_coeffects ?expected env e =
  Env.with_origin2 env Decl_counters.Body (fun env ->
      Expr.expr_with_pure_coeffects ~expected env e)

let stmt env st =
  Env.with_origin env Decl_counters.Body (fun env -> Stmt.stmt env st)

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
open Typing_defs
module Env = Tast_env
module TCO = TypecheckerOptions
module MakeType = Typing_make_type
module SN = Naming_special_names
open String.Replace_polymorphic_compare

let should_enforce env = TCO.disallow_invalid_arraykey (Env.get_tcopt env)

let equiv_ak_inter_dyn env ty_expect =
  let r = get_reason ty_expect in
  let ak_dyn =
    MakeType.intersection r [MakeType.arraykey r; MakeType.dynamic r]
  in
  Env.can_subtype env ty_expect ak_dyn && Env.can_subtype env ak_dyn ty_expect

(* For new-inference, types of keys in collection types may not be resolved
 * until TAST check time. For this reason, we replicate some of the checks that
 * are applied in Typing_array_access.array_get here. Specifically, those
 * checks that involve the key type from a dictionary-like container, checked
 * against the type of the index expression. For example,
 * the container might have a type dict<k,t> and the index expression might
 * have a type k'. We want k' <: k.
 *)
let rec array_get ~array_pos ~expr_pos ~index_pos env array_ty index_ty =
  let type_index ?(is_covariant_index = false) env ty_have ty_expect reason =
    let t_env = Env.tast_env_as_typing_env env in
    let got_error =
      if
        Typing_env_types.(
          TypecheckerOptions.enable_sound_dynamic t_env.genv.tcopt)
      then
        let (_env, ty_err_opt) =
          Typing_coercion.coerce_type
            ~coerce_for_op:true
            index_pos
            reason
            t_env
            ty_have
            ty_expect
            Enforced
            Typing_error.Callback.index_type_mismatch
        in
        Option.is_some ty_err_opt
      else
        Option.is_none
          (Typing_coercion.try_coerce ~coerce:None t_env ty_have ty_expect)
    in
    if not got_error then
      Ok ()
    else if
      (Env.can_subtype env ty_have (MakeType.dynamic (get_reason ty_have))
      (* Terrible heuristic to agree with legacy: if we inferred `nothing` for
       * the key type of the array, just let it pass *)
      || Env.can_subtype env ty_expect (MakeType.nothing (get_reason ty_expect))
      )
      (* If the key is not even an arraykey, we've already produced an error *)
      || (not (Env.can_subtype env ty_have (MakeType.arraykey Reason.Rnone)))
         && should_enforce env
      (* Keytype of arraykey&dynamic happens when you assign a dynamic into a dict,
         but the above coercion doesn't work. *)
      || equiv_ak_inter_dyn env ty_expect
    then
      Ok ()
    else
      let reasons_opt =
        Some
          (lazy
            (let (_, ty_have) = Env.expand_type env ty_have in
             let (_, ty_expect) = Env.expand_type env ty_expect in
             let ty_expect_str = Env.print_error_ty env ty_expect in
             let ty_have_str = Env.print_error_ty env ty_have in

             Typing_reason.to_string
               ("This is " ^ ty_expect_str)
               (get_reason ty_expect)
             @ Typing_reason.to_string
                 ("It is incompatible with " ^ ty_have_str)
                 (get_reason ty_have)))
      in
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          primary
          @@ Primary.Index_type_mismatch
               {
                 pos = expr_pos;
                 msg_opt = Some (Reason.string_of_ureason reason);
                 reasons_opt;
                 is_covariant_container = is_covariant_index;
               });
      Error ()
  in
  let (_, ety) = Env.expand_type env array_ty in
  match get_node ety with
  | Tunion tyl ->
    List.iter tyl ~f:(fun ty ->
        array_get ~array_pos ~expr_pos ~index_pos env ty index_ty)
  | Tclass ((_, cn), _, key_ty :: _)
    when cn = SN.Collections.cDict || cn = SN.Collections.cKeyset ->
    (* dict and keyset are both covariant in their key types so it is only
       required that the index be a subtype of the upper bound on that param, i.e., arraykey
       Here, we first check that we do not have a type error; if we do not, we
       then check that the indexing expression is a subtype of the key type and
       raise a distinct error.
       This allows us to retain the older behavior (which raised an index_type_mismatch error)
       whilst allowing us to discern between it and errors which can be
       addressed with `UNSAFE_CAST`
    *)
    let arraykey_ty = MakeType.arraykey (Reason.Ridx_dict array_pos) in
    let array_key_res =
      type_index env index_ty arraykey_ty (Reason.index_class cn)
    in
    (match array_key_res with
    | Error _ ->
      (* We have raised the more general error *)
      ()
    | Ok _ ->
      (* The index expression _is_ a subtype of arraykey, now check it is a subtype
         of the given key_ty *)
      let (_ : (unit, unit) result) =
        type_index
          ~is_covariant_index:true
          env
          index_ty
          key_ty
          (Reason.index_class cn)
      in
      ())
  | Tclass ((_, cn), _, key_ty :: _)
    when cn = SN.Collections.cMap
         || cn = SN.Collections.cConstMap
         || cn = SN.Collections.cImmMap
         || cn = SN.Collections.cKeyedContainer
         || cn = SN.Collections.cAnyArray ->
    let (_ : (unit, unit) result) =
      type_index env index_ty key_ty (Reason.index_class cn)
    in
    ()
  | Toption array_ty ->
    array_get ~array_pos ~expr_pos ~index_pos env array_ty index_ty
  | Tnonnull
  | Tprim _
  | Tfun _
  | Tvar _
  | Tclass _
  | Ttuple _
  | Tshape _
  | Tdynamic
  | Tany _
  | Tnewtype _
  | Tdependent _
  | _ ->
    ()

let index_visitor =
  object (this)
    inherit [_] Tast_visitor.iter_with_state as super

    (* Distinguish between lvalue and rvalue array indexing.
     * Suppose $x : dict<string,int>
     * We want to check rvalue e.g. $y = $x[3], or $z[$x[3]] = 5
     * But not lvalue e.g. $x[3] = 5 or list ($x[3], $w) = e;
     *)
    method! on_expr (env, is_lvalue) ((_, p, expr) as e) =
      match expr with
      | Array_get (((ty1, p1, _) as e1), Some ((ty2, p2, _) as e2)) ->
        if not is_lvalue then
          array_get ~array_pos:p1 ~expr_pos:p ~index_pos:p2 env ty1 ty2;
        this#on_expr (env, false) e1;
        this#on_expr (env, false) e2
      | Binop { bop = Ast_defs.Eq _; lhs; rhs } ->
        this#on_expr (env, true) lhs;
        this#on_expr (env, false) rhs
      | List el -> List.iter ~f:(this#on_expr (env, is_lvalue)) el
      | _ -> super#on_expr (env, is_lvalue) e
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env = index_visitor#on_fun_def (env, false)

    method! at_method_ env = index_visitor#on_method_ (env, false)
  end

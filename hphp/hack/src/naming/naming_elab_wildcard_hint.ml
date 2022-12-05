(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env : sig
  type t

  val empty : t

  val allow_wildcard : t -> bool

  val tp_depth : t -> int

  val incr_tp_depth : t -> t

  val reset_tp_depth : t -> t

  val set_allow_wildcard : t -> allow_wildcard:bool -> t
end = struct
  type t = {
    allow_wildcard: bool;
    tp_depth: int;
  }

  let empty = { allow_wildcard = false; tp_depth = 0 }

  let incr_tp_depth t = { t with tp_depth = t.tp_depth + 1 }

  let reset_tp_depth t = { t with tp_depth = 0 }

  let allow_wildcard { allow_wildcard; _ } = allow_wildcard

  let set_allow_wildcard t ~allow_wildcard = { t with allow_wildcard }

  let tp_depth { tp_depth; _ } = tp_depth
end

let on_expr_ (env, expr_, err) =
  let open Naming_phase_pass.Cont in
  let env =
    match expr_ with
    | Aast.Cast _ -> Env.incr_tp_depth env
    | Aast.(Is _ | As _) -> Env.set_allow_wildcard env ~allow_wildcard:true
    | Aast.Upcast _ -> Env.set_allow_wildcard env ~allow_wildcard:false
    | _ -> env
  in
  next (env, expr_, err)

let on_targ (env, targ, err) =
  let open Naming_phase_pass.Cont in
  next
    ( Env.set_allow_wildcard ~allow_wildcard:true @@ Env.incr_tp_depth env,
      targ,
      err )

let on_hint_ (env, hint_, err) =
  let open Naming_phase_pass.Cont in
  let env =
    match hint_ with
    | Aast.(
        ( Hunion _ | Hintersection _ | Hoption _ | Hlike _ | Hsoft _
        | Hrefinement _ )) ->
      Env.reset_tp_depth env
    | Aast.(Htuple _ | Happly _ | Habstr _ | Hvec_or_dict _) ->
      Env.incr_tp_depth env
    | _ -> env
  in
  next (env, hint_, err)

let on_shape_field_info (env, sfi, err) =
  let open Naming_phase_pass.Cont in
  next (Env.incr_tp_depth env, sfi, err)

let on_context (env, hint, err_acc) =
  let open Naming_phase_pass.Cont in
  match hint with
  | (pos, Aast.Happly ((_, tycon_name), _))
    when String.equal tycon_name SN.Typehints.wildcard ->
    let err = Err.naming @@ Naming_error.Invalid_wildcard_context pos in
    finish (env, (pos, Aast.Herr), Err.Free_monoid.plus err_acc err)
  | _ -> next (env, hint, err_acc)

let on_hint (env, hint, err_acc) =
  let open Naming_phase_pass.Cont in
  match hint with
  | (pos, Aast.Happly ((_, tycon_name), hints))
    when String.equal tycon_name SN.Typehints.wildcard ->
    if Env.(allow_wildcard env && tp_depth env >= 1) (* prevents 3 as _ *) then
      if not (List.is_empty hints) then
        let err =
          Err.naming
          @@ Naming_error.Tparam_applied_to_type
               { pos; tparam_name = SN.Typehints.wildcard }
        in
        finish (env, (pos, Aast.Herr), Err.Free_monoid.plus err_acc err)
      else
        next (env, hint, err_acc)
    else
      let err = Err.naming @@ Naming_error.Wildcard_hint_disallowed pos in
      finish (env, (pos, Aast.Herr), Err.Free_monoid.plus err_acc err)
  | _ -> next (env, hint, err_acc)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_expr_ = Some on_expr_;
        on_targ = Some on_targ;
        on_hint_ = Some on_hint_;
        on_shape_field_info = Some on_shape_field_info;
        on_context = Some on_context;
        on_hint = Some on_hint;
      })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem

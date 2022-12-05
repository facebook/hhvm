(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env : sig
  type t

  val empty : t

  val allow_like : t -> bool

  val set_allow_like : t -> allow_like:bool -> t
end = struct
  type t = { allow_like: bool }

  let empty = { allow_like = false }

  let set_allow_like _ ~allow_like = { allow_like }

  let allow_like { allow_like } = allow_like
end

let on_expr_ (env, expr_, err) =
  let env =
    match expr_ with
    | Aast.(Is _ | As _ | Upcast _) -> Env.set_allow_like env ~allow_like:true
    | _ -> env
  in
  Naming_phase_pass.Cont.next (env, expr_, err)

let on_hint_ (env, hint_, err) =
  let env =
    match hint_ with
    | Aast.(Hfun _ | Happly _ | Haccess _ | Habstr _ | Hvec_or_dict _) ->
      Env.set_allow_like env ~allow_like:false
    | _ -> env
  in
  Naming_phase_pass.Cont.next (env, hint_, err)

let on_hint (env, hint, err) =
  let err =
    match hint with
    | (pos, Aast.Hlike _) when not @@ Env.allow_like env ->
      Err.Free_monoid.plus err @@ Err.like_type pos
    | _ -> err
  in
  Naming_phase_pass.Cont.next (env, hint, err)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_expr_ = Some on_expr_;
        on_hint_ = Some on_hint_;
        on_hint = Some on_hint;
      })

let visitor = Naming_phase_pass.mk_visitor [pass]

let validate f ?init ?(env = Env.empty) elem =
  Err.from_monoid ?init @@ snd @@ f env elem

let validate_program ?init ?env elem =
  validate visitor#on_program ?init ?env elem

let validate_module_def ?init ?env elem =
  validate visitor#on_module_def ?init ?env elem

let validate_class ?init ?env elem = validate visitor#on_class_ ?init ?env elem

let validate_typedef ?init ?env elem =
  validate visitor#on_typedef ?init ?env elem

let validate_fun_def ?init ?env elem =
  validate visitor#on_fun_def ?init ?env elem

let validate_gconst ?init ?env elem = validate visitor#on_gconst ?init ?env elem

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
end = struct
  type t = unit

  let empty = ()
end

let on_expr_ (env, expr_, err_acc) =
  let err =
    match expr_ with
    | Aast.(Cast ((_, Hprim (Tint | Tbool | Tfloat | Tstring)), _)) -> err_acc
    | Aast.(Cast ((_, Happly ((_, tycon_nm), _)), _))
      when String.(
             equal tycon_nm SN.Collections.cDict
             || equal tycon_nm SN.Collections.cVec) ->
      err_acc
    | Aast.(Cast ((_, Aast.Hvec_or_dict (_, _)), _)) -> err_acc
    | Aast.(Cast ((_, Aast.Hany), _)) ->
      (* We end up with a `Hany` when we have an arity error for dict/vec
         - we don't error on this case to preserve behaviour
      *)
      err_acc
    | Aast.(Cast ((pos, _), _)) ->
      Err.Free_monoid.plus err_acc @@ Err.naming @@ Naming_error.Object_cast pos
    | _ -> err_acc
  in
  Naming_phase_pass.Cont.next (env, expr_, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_expr_ = Some on_expr_ })

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

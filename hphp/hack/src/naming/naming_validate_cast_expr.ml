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

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.reduce

    inherit Err.monoid

    method! on_Cast env hint expr =
      let expr_err = self#on_expr env expr in
      let cast_hint_err =
        match hint with
        | (_, Aast.(Hprim (Tint | Tbool | Tfloat | Tstring))) -> self#zero
        | (_, Aast.(Happly ((_, tycon_nm), _)))
          when String.(
                 equal tycon_nm SN.Collections.cDict
                 || equal tycon_nm SN.Collections.cVec) ->
          self#zero
        | (_, Aast.Hvec_or_dict (_, _)) -> self#zero
        | (_, Aast.Hany) ->
          (* We end up with a `Hany` when we have an arity error for dict/vec
             - we don't error on this case to preserve behaviour
          *)
          self#zero
        | (pos, _) -> Err.naming @@ Naming_error.Object_cast pos
      in
      self#plus expr_err cast_hint_err
  end

let validate f ?init ?(env = Env.empty) elem =
  Err.from_monoid ?init @@ f env elem

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

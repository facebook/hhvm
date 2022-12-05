(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  type t = { allow_retonly: bool }

  let empty = { allow_retonly = false }
end

let visitor =
  object (_self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_hint (Env.{ allow_retonly } as env) hint =
      let res =
        match hint with
        | (pos, Aast.(Hprim Tvoid)) when not allow_retonly ->
          Error
            ( (pos, Aast.Herr),
              Err.naming
              @@ Naming_error.Return_only_typehint { pos; kind = `void } )
        | (pos, Aast.(Hprim Tnoreturn)) when not allow_retonly ->
          Error
            ( (pos, Aast.Herr),
              Err.naming
              @@ Naming_error.Return_only_typehint { pos; kind = `noreturn } )
        | (_, Aast.(Happly _ | Habstr _)) ->
          Ok (Env.{ allow_retonly = true }, hint)
        | _ -> Ok (env, hint)
      in
      match res with
      | Ok (env, hint) -> super#on_hint env hint
      | Error (hint, err) -> (hint, err)

    method! on_targ _ targ = super#on_targ Env.{ allow_retonly = true } targ

    method! on_hint_fun_hf_return_ty _ t =
      super#on_hint_fun_hf_return_ty Env.{ allow_retonly = true } t

    method! on_fun_f_ret _ f = super#on_fun_f_ret Env.{ allow_retonly = true } f

    method! on_method_m_ret _ m =
      super#on_method_m_ret Env.{ allow_retonly = true } m
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem

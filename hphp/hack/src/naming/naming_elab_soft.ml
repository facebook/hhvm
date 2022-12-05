(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Env = struct
  type t = bool

  let empty = false
end

let visitor =
  object (_self)
    inherit [_] Aast_defs.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_hint soft_as_like hint =
      let hint =
        match hint with
        | (pos, Aast.Hsoft soft_hint) ->
          let soft_hint = super#on_hint soft_as_like soft_hint in
          let hint_ =
            if soft_as_like then
              Aast.Hlike soft_hint
            else
              (* TODO[mjt] are we intentionally stripping `Hsoft` here? *)
              snd soft_hint
          in
          (pos, hint_)
        | _ -> hint
      in
      super#on_hint soft_as_like hint
  end

let elab f ?(env = Env.empty) elem = f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem

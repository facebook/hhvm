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

    method! on_hint soft_as_like ((pos, hint_) as hint) =
      match hint_ with
      | Aast.Hsoft soft_hint ->
        let soft_hint = super#on_hint soft_as_like soft_hint in
        let hint_ =
          if soft_as_like then
            Aast.Hlike soft_hint
          else
            (* TODO[mjt] are we intentionally stripping `Hsoft` here? *)
            snd soft_hint
        in
        (pos, hint_)
      | _ -> super#on_hint soft_as_like hint
  end

let elab_fun_def ?(env = Env.empty) fd = visitor#on_fun_def env fd

let elab_typedef ?(env = Env.empty) td = visitor#on_typedef env td

let elab_module_def ?(env = Env.empty) m = visitor#on_module_def env m

let elab_gconst ?(env = Env.empty) gc = visitor#on_gconst env gc

let elab_class ?(env = Env.empty) cls = visitor#on_class_ env cls

let elab_program ?(env = Env.empty) prog = visitor#on_program env prog

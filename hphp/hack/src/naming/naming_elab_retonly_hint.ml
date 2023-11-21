(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let allow_retonly
      Naming_phase_env.
        { elab_retonly_hint = Elab_retonly_hint.{ allow_retonly }; _ } =
    allow_retonly

  let set_allow_retonly t ~allow_retonly =
    Naming_phase_env.
      { t with elab_retonly_hint = Elab_retonly_hint.{ allow_retonly } }
end

let on_targ targ ~ctx =
  let ctx = Env.set_allow_retonly ctx ~allow_retonly:true in
  (ctx, Ok targ)

let on_hint_fun_hf_return_ty t ~ctx =
  let ctx = Env.set_allow_retonly ctx ~allow_retonly:true in
  (ctx, Ok t)

let on_fun_f_ret f ~ctx =
  let ctx = Env.set_allow_retonly ctx ~allow_retonly:true in
  (ctx, Ok f)

let on_method_m_ret m ~ctx =
  let ctx = Env.set_allow_retonly ctx ~allow_retonly:true in
  (ctx, Ok m)

let on_hint_ hint_ ~ctx =
  match hint_ with
  | Aast.(Happly _ | Habstr _) ->
    let ctx = Env.set_allow_retonly ctx ~allow_retonly:true in
    (ctx, Ok hint_)
  | _ -> (ctx, Ok hint_)

let on_hint on_error hint ~ctx =
  let allow_retonly = Env.allow_retonly ctx in
  match hint with
  | (pos, Aast.(Hprim Tvoid)) when not allow_retonly ->
    on_error
      (Naming_phase_error.naming
         Naming_error.(Return_only_typehint { pos; kind = Hvoid }));
    (ctx, Error hint)
  | (pos, Aast.(Hprim Tnoreturn)) when not allow_retonly ->
    on_error
      (Naming_phase_error.naming
         Naming_error.(Return_only_typehint { pos; kind = Hnoreturn }));
    (ctx, Error hint)
  | _ -> (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_hint_ = Some on_hint_;
        on_ty_targ = Some on_targ;
        on_fld_hint_fun_hf_return_ty = Some on_hint_fun_hf_return_ty;
        on_fld_fun__f_ret = Some on_fun_f_ret;
        on_fld_method__m_ret = Some on_method_m_ret;
        on_ty_hint = Some (on_hint on_error);
      }

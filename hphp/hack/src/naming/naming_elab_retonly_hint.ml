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

let on_targ (env, targ, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, targ, err)

let on_hint_fun_hf_return_ty (env, t, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, t, err)

let on_fun_f_ret (env, f, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, f, err)

let on_method_m_ret (env, m, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, m, err)

let on_hint (env, hint, err_acc) =
  let allow_retonly = Env.allow_retonly env in
  let res =
    match hint with
    | (pos, Aast.(Hprim Tvoid)) when not allow_retonly ->
      Error
        ( (pos, Aast.Herr),
          Naming_phase_error.naming
          @@ Naming_error.Return_only_typehint { pos; kind = `void } )
    | (pos, Aast.(Hprim Tnoreturn)) when not allow_retonly ->
      Error
        ( (pos, Aast.Herr),
          Naming_phase_error.naming
          @@ Naming_error.Return_only_typehint { pos; kind = `noreturn } )
    | (_, Aast.(Happly _ | Habstr _)) ->
      let env = Env.set_allow_retonly env ~allow_retonly:true in
      Ok (env, hint)
    | _ -> Ok (env, hint)
  in
  match res with
  | Ok (env, hint) -> Ok (env, hint, err_acc)
  | Error (hint, err) -> Error (env, hint, err :: err_acc)

let pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_hint = Some on_hint;
        on_targ = Some on_targ;
        on_hint_fun_hf_return_ty = Some on_hint_fun_hf_return_ty;
        on_fun_f_ret = Some on_fun_f_ret;
        on_method_m_ret = Some on_method_m_ret;
      })

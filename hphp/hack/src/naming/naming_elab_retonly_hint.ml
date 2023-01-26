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

let on_targ (env, targ) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, targ)

let on_hint_fun_hf_return_ty (env, t) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, t)

let on_fun_f_ret (env, f) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, f)

let on_method_m_ret (env, m) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Ok (env, m)

let on_hint on_error (env, hint) =
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
  | Ok (env, hint) -> Ok (env, hint)
  | Error (hint, err) ->
    on_error err;
    Error (env, hint)

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_hint = Some (on_hint on_error);
          on_targ = Some on_targ;
          on_hint_fun_hf_return_ty = Some on_hint_fun_hf_return_ty;
          on_fun_f_ret = Some on_fun_f_ret;
          on_method_m_ret = Some on_method_m_ret;
        })

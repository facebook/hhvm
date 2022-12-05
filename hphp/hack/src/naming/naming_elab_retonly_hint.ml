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

  val allow_retonly : t -> bool

  val set_allow_retonly : t -> allow_retonly:bool -> t
end = struct
  type t = { allow_retonly: bool }

  let empty = { allow_retonly = false }

  let allow_retonly { allow_retonly } = allow_retonly

  let set_allow_retonly _ ~allow_retonly = { allow_retonly }
end

let on_targ (env, targ, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Naming_phase_pass.Cont.next (env, targ, err)

let on_hint_fun_hf_return_ty (env, t, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Naming_phase_pass.Cont.next (env, t, err)

let on_fun_f_ret (env, f, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Naming_phase_pass.Cont.next (env, f, err)

let on_method_m_ret (env, m, err) =
  let env = Env.set_allow_retonly env ~allow_retonly:true in
  Naming_phase_pass.Cont.next (env, m, err)

let on_hint (env, hint, err_acc) =
  let allow_retonly = Env.allow_retonly env in
  let res =
    match hint with
    | (pos, Aast.(Hprim Tvoid)) when not allow_retonly ->
      Error
        ( (pos, Aast.Herr),
          Err.naming @@ Naming_error.Return_only_typehint { pos; kind = `void }
        )
    | (pos, Aast.(Hprim Tnoreturn)) when not allow_retonly ->
      Error
        ( (pos, Aast.Herr),
          Err.naming
          @@ Naming_error.Return_only_typehint { pos; kind = `noreturn } )
    | (_, Aast.(Happly _ | Habstr _)) ->
      let env = Env.set_allow_retonly env ~allow_retonly:true in
      Ok (env, hint)
    | _ -> Ok (env, hint)
  in
  match res with
  | Ok (env, hint) -> Naming_phase_pass.Cont.next (env, hint, err_acc)
  | Error (hint, err) ->
    Naming_phase_pass.Cont.finish (env, hint, Err.Free_monoid.plus err_acc err)

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

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Typing_defs
module A = Aast_defs
module TUtils = Typing_utils
module Env = Tast_env

let is_always_castable env ty =
  let open Typing_make_type in
  let r = Reason.Rnone in
  let mixed = mixed r in
  let castable_ty =
    nullable_locl
      r
      (union r [arraykey r; num r; bool r; hh_formatstring r mixed])
  in
  Env.is_sub_type env ty castable_ty

let is_bool_castable env ty =
  let open Typing_make_type in
  let r = Reason.Rnone in
  let mixed = mixed r in
  let bool_castable_ty =
    nullable_locl
      r
      (union r [vec_or_dict r (arraykey r) mixed; keyset r (arraykey r)])
  in
  Env.is_sub_type env ty bool_castable_ty

let is_not_castable_but_leads_to_too_many_false_positives env ty =
  let (env, ty) = Env.expand_type env ty in
  let open Typing_make_type in
  let r = Reason.Rnone in
  let nonnull = nonnull r in
  let dynamic = dynamic r in
  Env.is_sub_type env nonnull ty
  || Env.is_sub_type env dynamic ty
  ||
  match T.get_node ty with
  | T.Tgeneric _
  | T.Tnewtype _ ->
    true
  | _ -> false

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      let typing_env = Env.tast_env_as_typing_env env in
      function
      | (_, pos, Aast.Cast (hint, (ty, _, _)))
        when (not (TUtils.is_nothing typing_env ty))
             && (not (is_always_castable env ty))
             && (not
                   (is_bool_castable env ty
                   && A.(equal_hint_ (snd hint) (Hprim Tbool))))
             && not
                  (is_not_castable_but_leads_to_too_many_false_positives env ty)
        ->
        Lints_errors.cast_non_primitive pos
      | _ -> ()
  end

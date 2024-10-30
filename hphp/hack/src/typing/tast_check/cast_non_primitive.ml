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

let warning_kind = Typing_warning.Cast_non_primitive

let error_codes = Typing_warning_utils.codes warning_kind

let is_always_castable env ty =
  let open Typing_make_type in
  let r = Reason.none in
  let mixed = mixed r in
  let castable_ty =
    locl_like r
    @@ nullable r
    @@ union r [arraykey r; num r; bool r; hh_formatstring r mixed]
  in
  Env.is_sub_type env ty castable_ty

let is_bool_castable env ty =
  let open Typing_make_type in
  let r = Reason.none in
  let mixed = mixed r in
  let bool_castable_ty =
    locl_like r
    @@ nullable r
    @@ union r [vec_or_dict r (arraykey r) mixed; keyset r (arraykey r)]
  in
  Env.is_sub_type env ty bool_castable_ty

let is_not_castable_but_leads_to_too_many_false_positives env ty =
  let (env, ty) = Env.expand_type env ty in
  let open Typing_make_type in
  let r = Reason.none in
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

(** This linter catches dangerous uses of `(int)`, `(string)`, `(bool)`, and
    `(float)` casts. These are safe to perform on the following inductive set
    - primitives
    - format strings
    - nullables of safe types to perform casts.

    Additionally, collections can be cast using `(bool)`.

    Although, it is not known to be safe to perform casts on `mixed`,
    `dynamic`, `nothing`, or genericly typed values, we do not lint against
    them to prevent excessive false positives.
  *)
let handler ~as_lint =
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
        Typing_warning_utils.add_for_migration
          (Env.get_tcopt env)
          ~as_lint:
            (if as_lint then
              Some None
            else
              None)
          ( pos,
            Typing_warning.Cast_non_primitive,
            {
              Typing_warning.Cast_non_primitive.cast_hint =
                Env.print_hint env hint;
              ty = Env.print_ty env ty;
            } )
      | _ -> ()
  end

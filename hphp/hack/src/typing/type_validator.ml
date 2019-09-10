(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module Env = Tast_env
module MakeType = Typing_make_type
module Reason = Typing_reason

type validity =
  | Valid
  | Invalid : Reason.t * string -> validity

type validation_state = {
  env: Env.env;
  ety_env: expand_env;
  validity: validity;
  like_context: bool;
}

class virtual type_validator =
  object (this)
    inherit [validation_state] Type_visitor.type_visitor

    method validate_type env root_ty emit_error =
      let should_suppress = ref false in
      let validate env ety_env ty =
        let state =
          this#on_type
            { env; ety_env; validity = Valid; like_context = false }
            ty
        in
        match state.validity with
        | Invalid (r, msg) ->
          if not !should_suppress then
            emit_error (Reason.to_pos (fst root_ty)) (Reason.to_pos r) msg;
          should_suppress := true
        | Valid -> ()
      in
      let (env, root_ty) =
        Env.localize_with_dty_validator env root_ty (validate env)
      in
      validate
        env
        {
          type_expansions = [];
          substs = SMap.empty;
          this_ty =
            Option.value
              (Env.get_self env)
              ~default:(MakeType.nothing Reason.none);
          from_class = None;
          validate_dty = None;
        }
        root_ty

    method validate_hint env hint emit_error =
      let hint_ty = Env.hint_to_ty env hint in
      this#validate_type env hint_ty emit_error

    method invalid state r msg =
      if state.validity = Valid then
        { state with validity = Invalid (r, msg) }
      else
        state
  end

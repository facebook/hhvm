(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module MakeType = Typing_make_type
module SN = Naming_special_names

let rxTraversableType =
  MakeType.class_type
    Reason.none
    SN.Rx.cTraversable
    [MakeType.mixed Reason.Rnone]

let rxAsyncIteratorType =
  MakeType.class_type
    Reason.none
    SN.Rx.cAsyncIterator
    [MakeType.mixed Reason.Rnone]

let check_foreach_collection env p ty =
  (* do nothing if unsafe_rx is set *)
  if TypecheckerOptions.unsafe_rx (Env.get_tcopt env) then
    ()
  else
    match Env.env_reactivity env with
    | Nonreactive
    | Local _ ->
      ()
    | _ ->
      let rec check ty =
        let (env, ty) = Env.expand_type env ty in
        match get_node ty with
        | Tunion l -> List.for_all l ~f:check
        | _ ->
          (* collection type should be subtype or conditioned to Rx\Traversable *)
          if
            not
              ( Env.can_subtype env ty rxTraversableType
              || Env.can_subtype env ty rxAsyncIteratorType
              || Env.condition_type_matches
                   ~is_self:false
                   env
                   ty
                   rxTraversableType
              || Env.condition_type_matches
                   ~is_self:false
                   env
                   ty
                   rxAsyncIteratorType )
          then (
            Errors.invalid_traversable_in_rx p;
            false
          ) else
            true
      in
      ignore (check ty)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env x =
      match snd x with
      | Foreach (((p, ty), _), _, _) -> check_foreach_collection env p ty
      | _ -> ()
  end

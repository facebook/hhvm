(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

module Reason  = Typing_reason
module TUtils  = Typing_utils
module Env     = Typing_env
module Inst    = Typing_instantiate
module Unify   = Typing_unify
module TDef    = Typing_tdef
module SubType = Typing_subtype

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

let sub_type p ur env ty1 ty2 =
  let env = { env with Env.pos = p } in
  Errors.try_
    (fun () -> SubType.sub_type env ty1 ty2)
    (fun l ->
      Errors.add_list ((p, Reason.string_of_ureason ur) :: l);
      env
    )

let unify p ur env ty1 ty2 =
  let env = { env with Env.pos = p } in
  Errors.try_ 
    (fun () -> Unify.unify env ty1 ty2)
    (fun l ->
      Errors.add_list ((p, Reason.string_of_ureason ur) :: l);
      env, (Reason.Rwitness p, Tany)
    )


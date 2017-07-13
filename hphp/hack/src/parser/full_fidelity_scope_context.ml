(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TokenKind = Full_fidelity_token_kind

type t = {
  expected : TokenKind.t list
}

let empty =
  { expected = [ ] }

let make token_kind_list =
  { expected = token_kind_list }

let expected scope_context =
  scope_context.expected

(* Given the previous context and a newly-expected token_kind,
 * add the new token_kind to the front of the previous expected stack. *)
let expect scope_context token_kind =
  { expected = token_kind :: scope_context.expected }

(* Given a ScopeContext.t, test if it expects token_kind. *)
(* NOTE that this is an O(n) search. Usually n is small, so we can probably
 * get away with this for now, but it might be worth optimizing out later. *)
let expects scope_context token_kind =
  let matches_given_kind k = (k = token_kind) in
  Core.List.exists scope_context.expected ~f:matches_given_kind

let expects_next scope_context token_kind =
  match scope_context.expected with
  | h :: t -> h = token_kind
  | _ -> false

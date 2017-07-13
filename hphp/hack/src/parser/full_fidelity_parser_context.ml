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
module ScopeContext = Full_fidelity_scope_context

type t = {
  expected : ScopeContext.t list
}

let empty =
  { expected = [ ] }

let expected context =
  context.expected

(* Given a Context.t, returns the expected TokenKind.t list of its
 * lowest context. *)
let lowest_scope_expected context =
  match context.expected with
  | h :: _ -> ScopeContext.expected h
  | [ ] -> [ ]

(* Given a current context, test if it expects token_kind. *)
(* NOTE that this is an O(n) search. Usually n is small, so we can probably
 * get away with this for now, but it might be worth optimizing out later. *)
let expects context token_kind =
  let expects_kind scope_context =
    ScopeContext.expects scope_context token_kind in
  Core.List.exists context.expected ~f:expects_kind

(* Tests if a given token_kind is expected in the lowest scope. *)
let lowest_scope_expects context token_kind =
  match context.expected with
  | h :: _ -> ScopeContext.expects h token_kind
  | [ ] -> false (* Wholly empty contexts clearly don't expect token_kind *)

(* Given a context and a token_kind, add it to the front of the lowest
 * existing scope, or create a new scope for it if none exists yet. *)
(* NOTE that in contrast to expect_in_new_scope, this function takes a single
 * token_kind, not a TokenKind.t list. *)
let expect context token_kind =
  match context.expected with
  | h :: t -> { expected = (ScopeContext.expect h token_kind) :: t }
  | [ ] -> { expected = [ ScopeContext.make [ token_kind ] ] }

let expect_in_new_scope context token_kind_list =
  let new_scope = ScopeContext.make token_kind_list in
  { expected =  new_scope :: context.expected }

(* Check if token_kind is next in the parser's list of expected token kinds. *)
let expects_next context token_kind =
  match context.expected with
  | h :: _ -> ScopeContext.expects_next h token_kind
  | [ ] -> false

(* Access the non-scoped expected stack: returns a TokenKind.t list. *)
let expected_flattened context =
  let unwrapped_scopes = List.map ScopeContext.expected context.expected in
  List.flatten unwrapped_scopes

(* Utility function for debugging. Prints the flattened list of
 * expected tokens. *)
let print_expected_list context =
  let expected_list = expected_flattened context in
  Printf.printf "The list of expected token kinds has %d elements. They are:\n"
    (Core.List.length expected_list);
  let print_token_kind tok_kind = Printf.printf "%s\n"
    (TokenKind.to_string tok_kind); in
  Core.List.map expected_list ~f:print_token_kind

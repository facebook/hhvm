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
  expected : TokenKind.t list;
}

let empty =
  let expected = [ ] in (* Always start the context empty *)
  { expected }

(* Given the previous context and a newly-expected token_kind,
 * add the new token_kind to the front of the previous expected stack. *)
let expect context token_kind =
  { expected = token_kind :: context.expected }

(* Given a current context, test if it expects token_kind. *)
(* NOTE that this is an O(n) search. Usually n is small, so we can probably
 * get away with this for now, but it might be worth optimizing out later. *)
let expects context token_kind =
  Core.List.exists context.expected ~f:(fun k -> (k = token_kind))

(* Check if token_kind is next in the parser's list of expected token kinds. *)
let expects_next context token_kind =
  match context.expected with
  | h :: t -> h = token_kind
  | _ -> false

(* Access the expected stack of a context. *)
let expected context =
  context.expected

(* Utility function for debugging. *)
let print_expected_list context =
  let expected_list = context.expected in
  Printf.printf "The list of expected token kinds has %d elements. They are:\n"
    (Core.List.length expected_list);
  let print_token_kind tok_kind = Printf.printf "%s\n"
    (TokenKind.to_string tok_kind); in
  Core.List.map expected_list ~f:print_token_kind

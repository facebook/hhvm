(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

 (**
 * This is a stack of scopes, and a scope is a set of expected `TokenKind`s.
 * The top of the stack is the "current" scope. If we come across a `TokenKind`
 * that is not in the current scope, but does occur in scopes further down
 * the stack, we assume that the expected tokens were erroneously omitted in
 * the input. In that case we must inform the user of their mistake by
 * presenting an informative error message, but we must also continue
 * parsing to the best of our ability.
 *)

open Core_kernel

module TokenKind = Full_fidelity_token_kind

module Scope = struct
  include Caml.Set.Make(struct
    type t = TokenKind.t
    let compare = Pervasives.compare
    end)
  let to_string scope =
    let append elt acc = acc ^ "'" ^ (TokenKind.to_string elt) ^ "', " in
    fold append scope ">> "
  let pp _formatter scope = to_string scope
end

module WithToken(Token: Lexable_token_sig.LexableToken_S) = struct

type t = {
  expected : Scope.t list;
  skipped_tokens : Token.t list
} [@@deriving show]

let empty =
  { expected = [ ]; skipped_tokens = [ ] }

let skipped_tokens context =
  context.skipped_tokens

let with_skipped_tokens context skipped_tokens =
  { context with skipped_tokens }

let with_expected context expected =
  { context with expected }

let expects context token_kind =
  List.exists context.expected
    ~f:(fun scope -> Scope.mem token_kind scope)

let expects_here context token_kind =
  match context.expected with
  | current :: _others -> Scope.mem token_kind current
  | [ ] -> false

let expect context token_kind_list =
  let scope_addendum = Scope.of_list token_kind_list in
  match context.expected with
  | current :: other ->
    let new_expected = Scope.union scope_addendum current :: other in
    with_expected context new_expected
  | [ ] -> with_expected context [ scope_addendum ]

let expect_in_new_scope context token_kind_list =
  let new_expected = Scope.of_list token_kind_list :: context.expected in
  with_expected context new_expected

(* Removes the top scope from the expected if and only if it
 * contains exactly the same elements as the given token_kind_list. *)
let pop_scope context token_kind_list =
  match context.expected with
  | current :: others
    when Scope.equal current (Scope.of_list token_kind_list) ->
    with_expected context others
  | [ ] when List.is_empty token_kind_list ->
    with_expected context [ ]
  (* Failure conditions *)
  | current :: _others ->
    let failure_str = Printf.sprintf
    ("Error: attempted to pop a list of token kinds that wasn't on top of " ^^
    "the context. Tried to pop: %sbut the first scope on the stack was " ^^
    "actually: %s") (Scope.to_string (Scope.of_list token_kind_list))
    (Scope.to_string current) in
    failwith (failure_str)
  | [ ] -> failwith ("Error: tried to pop a list of token kinds off of an " ^
    "empty context.")

(* Utility debugging function *)

let print_expected context =
  let expected = context.expected in
  Printf.printf "There are %d scopes on the stack. %!" (List.length expected);
  Printf.printf "The tokens we are expecting, from sooner to later, are:\n%!";
  let print_token_kind token_kind =
    Printf.printf "'%s', %!" (TokenKind.to_string token_kind); in
  let print_scope scope =
    Printf.printf ">> %!";
    Scope.iter print_token_kind scope;
    Printf.printf "\n%!"; in
  List.iter expected print_scope

end (* WithToken *)

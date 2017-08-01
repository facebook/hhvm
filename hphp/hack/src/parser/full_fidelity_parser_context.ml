(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

module TokenKind = Full_fidelity_token_kind
module MinimalTrivia = Full_fidelity_minimal_trivia
module MinimalToken = Full_fidelity_minimal_token
module TriviaKind = Full_fidelity_trivia_kind

module Scope = struct
  include Set.Make(struct
    type t = TokenKind.t
    let compare = Pervasives.compare
    end)
  let to_string scope =
    let append elt acc = acc ^ "'" ^ (TokenKind.to_string elt) ^ "', " in
    fold append scope ">> "
end

type t = {
  expected : Scope.t list;
  extra_token_error : MinimalTrivia.t list option
}

let empty =
  { expected = [ ]; extra_token_error = None }

let with_expected context expected =
  { context with expected }

let expects context token_kind =
  Core.List.exists context.expected
    ~f:(fun scope -> Scope.mem token_kind scope)

let expects_here context token_kind =
  match context.expected with
  | current :: others -> Scope.mem token_kind current
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
  | [ ] when Core.List.is_empty token_kind_list ->
    with_expected context [ ]
  (* Failure conditions *)
  | current :: others ->
    let failure_str = Printf.sprintf
    ("Error: attempted to pop a list of token kinds that wasn't on top of " ^^
    "the context. Tried to pop: %sbut the first scope on the stack was " ^^
    "actually: %s") (Scope.to_string (Scope.of_list token_kind_list))
    (Scope.to_string current) in
    failwith (failure_str)
  | [ ] -> failwith ("Error: tried to pop a list of token kinds off of an " ^
    "empty context.")

(* Dealing with extra_token_error *)

let carry_extra context token =
  match token with
  | None -> { expected = (context.expected); extra_token_error = None }
  | Some token ->
    let extra_token_error = Some (MinimalToken.as_error_trivia_list token) in
    {context with extra_token_error}

let carrying_extra context =
  Option.is_some context.extra_token_error

let flush_extra context =
  match context.extra_token_error with
  | Some extra -> (carry_extra context None, extra)
  | None -> failwith("Error: tried to flush an extra token from parser " ^
    "context, but parser context was not carrying an extra token.")

(* Utility debugging function *)

let print_expected context =
  let expected = context.expected in
  Printf.printf "There are %d scopes on the stack. %!" (Core.List.length expected);
  Printf.printf "The tokens we are expecting, from sooner to later, are:\n%!";
  let print_token_kind token_kind =
    Printf.printf "'%s', %!" (TokenKind.to_string token_kind); in
  let print_scope scope =
    Printf.printf ">> %!";
    Scope.iter print_token_kind scope;
    Printf.printf "\n%!"; in
  Core.List.iter expected print_scope

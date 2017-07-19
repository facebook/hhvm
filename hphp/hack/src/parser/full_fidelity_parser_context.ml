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

module Scope = struct
  include Set.Make(struct
    type t = TokenKind.t
    let compare = Pervasives.compare
    end)
end

type t = Scope.t list

let empty =
  [ ]

let expects context token_kind =
  Core.List.exists context ~f:(fun scope -> Scope.mem token_kind scope)

let expects_here context token_kind =
  expects (Core.List.take context 1) token_kind

let expect context token_kind_list =
  let scope_addendum = Scope.of_list token_kind_list in
  match context with
  | current :: other -> Scope.union scope_addendum current :: other
  | [ ] -> [ scope_addendum ]

let expect_in_new_scope context token_kind_list =
  Scope.of_list token_kind_list :: context

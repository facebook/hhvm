(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
val handler : Tast_visitor.handler

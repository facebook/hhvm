(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This module performs some "validity" checks on declared types in
    function and member signatures, extends, implements, uses, etc.
    - trivial syntactic errors:
        - writing ?nonnull instead of mixed
        - ?void, ?noreturn, ?mixed
        - Tuple<X, Y> instead of (X, Y))
    - unsatisfied constraints (e.g. C<bool> where C requires T as arraykey)
    - check hint well-kinded-ness
    - check correct usage of __Atom attribute *)

val fun_ : Typing_env_types.env -> Nast.fun_ -> unit

val class_ : Typing_env_types.env -> Nast.class_ -> unit

val typedef : Typing_env_types.env -> Nast.typedef -> unit

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This module checks wellformedness of type hints in the decls and bodies.
    Wellformedness checks include:
    - constraints on type parameters (e.g. C<string> where C requires T as arraykey)
    - hint well-kinded-ness
    - trivial syntactic errors:
        - writing ?nonnull instead of mixed
        - ?void, ?noreturn, ?mixed
        - Tuple<X, Y> instead of (X, Y))
    - correct usage of __ViaLabel attribute

    NB: this is akin to well-formedness in e.g.
    "Featherweight Java: A Minimal Core Calculus for Java and GJ", 2002,
    Igarashi, Pierce, Wadler. *)

val fun_ : Typing_env_types.env -> Nast.fun_ -> unit

val class_ : Typing_env_types.env -> Nast.class_ -> unit

val typedef : Typing_env_types.env -> Nast.typedef -> unit

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Evaluate [value] once, then compose bounded expressions and statements that
    preserve its type. Supplied operations must accept and produce [ty]. The enclosing context must provide [defaults]. *)
val compose :
  ty:string ->
  value:Milner_syntax.expr ->
  operations:(Milner_syntax.expr -> Milner_syntax.expr) list ->
  Milner_syntax.expr

(** Shuffle and interleave named items while preserving positional order. *)
val permute_named : ('a -> bool) -> 'a list -> 'a list

(** Choose named parameters and their declaration order once for a callable. *)
val promote_parameters :
  ?allow_named:bool ->
  Milner_syntax.parameter list ->
  Milner_syntax.parameter list

(** Plan a call against an existing signature. [None] omits an optional
    argument; positional holes before supplied arguments use their defaults.
    Variadic values are unpacked. Required arguments must be supplied. *)
val call_arguments :
  Milner_syntax.parameter list ->
  value:(Milner_syntax.parameter -> Milner_syntax.expr option) ->
  Milner_syntax.expr list

(** Apply a lambda, optionally promoting its parameters and arguments to named
    form and varying argument order. *)
val apply :
  ?contexts:string list ->
  string ->
  Milner_syntax.parameter list ->
  Milner_syntax.stmt list ->
  Milner_syntax.expr list ->
  Milner_syntax.expr

(** Apply a declared callable with the same named-argument choices as [apply].
    Materialization must place the declaration at top level. *)
val apply_declared :
  ?type_arguments:string list ->
  Milner_syntax.declaration ->
  Milner_syntax.expr list ->
  Milner_syntax.expr

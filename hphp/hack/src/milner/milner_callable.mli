(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Materialize closed callable bodies as auxiliary function and method
    definitions, retaining captures as lambdas. The unsafe option permits the
    declaration ordering excluded by T289079831. *)
val materialize :
  allow_unsafe_named_parameter_order:bool ->
  avoid_method_override:(is_async:bool -> return_hint:string -> bool) ->
  Milner_syntax.expr ->
  string list * Milner_syntax.expr

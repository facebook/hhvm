(** Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type RewritableType = sig
  type t

  val children : t -> t list

  val from_children :
    Full_fidelity_source_text.t ->
    int ->
    Full_fidelity_syntax_kind.t ->
    t list ->
    t

  val kind : t -> Full_fidelity_syntax_kind.t
end

module WithSyntax : functor (Syntax : RewritableType) -> sig
  module Result : sig
    type 'a t =
      | Remove
      | Keep
      | Replace of 'a
  end

  type 'a t = 'a Result.t =
    | Remove
    | Keep
    | Replace of 'a

  val parented_aggregating_rewrite_post :
    (Syntax.t list -> Syntax.t -> 'a -> 'a * Syntax.t t) ->
    Syntax.t ->
    'a ->
    'a * Syntax.t

  val aggregating_rewrite_post :
    (Syntax.t -> 'a -> 'a * Syntax.t t) -> Syntax.t -> 'a -> 'a * Syntax.t

  val parented_rewrite_post :
    (Syntax.t list -> Syntax.t -> Syntax.t t) -> Syntax.t -> Syntax.t

  val rewrite_post : (Syntax.t -> Syntax.t t) -> Syntax.t -> Syntax.t

  val parented_aggregating_rewrite_pre :
    (Syntax.t list -> Syntax.t -> 'a -> 'a * Syntax.t t) ->
    Syntax.t ->
    'a ->
    'a * Syntax.t

  val rewrite_pre : (Syntax.t -> Syntax.t t) -> Syntax.t -> Syntax.t

  val rewrite_pre_and_stop_with_acc :
    (Syntax.t -> 'a -> 'a * Syntax.t t) -> Syntax.t -> 'a -> 'a * Syntax.t

  val rewrite_pre_and_stop : (Syntax.t -> Syntax.t t) -> Syntax.t -> Syntax.t
end

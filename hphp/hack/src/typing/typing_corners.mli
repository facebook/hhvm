(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types

(** The key identifying a spread element *)
module Splat_elem : sig
  (** A localized generic or newtype used as a spread element key. *)
  type t = locl_ty

  (** Compare spread elements by normalized type identity and opacity. *)
  val compare : t -> t -> int

  (** Maps keyed by spread element. *)
  module Map : Stdlib.Map.S with type key = t

  (** Sets of spread elements. *)
  module Set : Stdlib.Set.S with type elt = t
end

(** Per-label corner assignments for spread elements. *)
module Assignment : sig
  (** What each spread element's field has been fixed to, at one label. *)
  type t = locl_phase shape_field_type Splat_elem.Map.t
end

(** Shape-field predicates and row-algebra operations. *)
module Field : sig
  (** A required or optional localized shape field. *)
  type t = locl_phase shape_field_type

  (** Whether the field must be present. *)
  val is_required : t -> bool

  (** Whether the field may be absent. *)
  val is_optional : t -> bool

  (** Whether the field is optional with type [nothing], hence always absent. *)
  val is_absent : t -> env -> bool

  (** [sub] is at least as required as [super]. *)
  val requiredness_lte : sub:t -> super:t -> bool

  (** Rightmost-wins merge of two fields. *)
  val merge : left:t -> right:t -> env -> env * t

  (** Greatest lower bound of two fields. *)
  val meet : left:t -> right:t -> env -> env * t

  (** Least upper bound of two fields. *)
  val join : left:t -> right:t -> env -> env * t

  (** Extremal field values induced by lower and upper bounds. *)
  module Corners : sig
    (** The field descriptor enumerated at each corner. *)
    type field = t

    (** The reachable corner fields, or an uninhabited inverted interval. *)
    type t =
      | Values of field list
      | Inverted

    (** Enumerate the distinct corners between [lower] and [upper]. *)
    val of_bounds : lower:field -> upper:field -> t
  end
end

(* == Rows ================================================================== *)

(** Read a row at one label under the current assignment. [None] is the label
    standing for every field name nobody wrote down. *)
val proj :
  env ->
  Typing_shape_normalize.Row.t ->
  TShapeField.t option ->
  Assignment.t ->
  env * locl_phase shape_field_type

(** Resolve a row to a single simple shape for reading a field, taking every
    live parameter to its upper bound. *)
val resolve_for_read : env -> Typing_reason.t -> locl_ty list -> env * locl_ty

(** Type parameter/newtype spreads that can affect [row] at [label], excluding
    those masked by a required field to their right; [None] selects the tail i.e.
    the upper bound for all unknown fields. *)
val row_live_spread_at :
  Typing_shape_normalize.Row.t -> TShapeField.t option -> locl_ty list

(* == Labels a comparison must cover ======================================== *)

(** Concrete labels contributed by either row or by bounds of their spreads. *)
val subrow_label_set :
  env ->
  sub:Typing_shape_normalize.Row.t ->
  super:Typing_shape_normalize.Row.t ->
  Typing_reason.t ->
  TShapeSet.t

(** The unknown tail ([None]) followed by every concrete label to compare. *)
val subrow_labels :
  env ->
  sub:Typing_shape_normalize.Row.t ->
  super:Typing_shape_normalize.Row.t ->
  Typing_reason.t ->
  TShapeField.t option list

(* == Ordering the parameters =============================================== *)

(** The order in which the given roots and everything reachable from them
    should be given values: each after those its bounds mention. Bounds can be
    cyclic, as [where T1 = T2] makes them, and then one dependency is dropped;
    the result is still total and does not vary between runs. *)
val topo : env -> Splat_elem.Set.t -> Typing_reason.t -> locl_ty list

(* == Corners =============================================================== *)

(** Check every corner assignment of the live spread elements. *)
val check_subrow_corners :
  env ->
  sub:Typing_shape_normalize.Row.t ->
  super:Typing_shape_normalize.Row.t ->
  TShapeField.t option ->
  Typing_reason.t ->
  init:(env -> env * 'a) ->
  conj:(env * 'a -> (env -> env * 'a) -> env * 'a) ->
  f:
    (env ->
    sub:locl_phase shape_field_type ->
    super:locl_phase shape_field_type ->
    env * 'a) ->
  env * 'a

(** Every corner co-assignment of the given spread elements. *)
val corner_assignments :
  env ->
  locl_ty list ->
  TShapeField.t option ->
  Typing_reason.t ->
  env * Assignment.t list

(* == Spread type variables ================================================= *)

(** Type-variable IDs occurring in spread position, in source order. *)
val spread_tyvar_ids : Typing_shape_normalize.Row.t -> Tvid.t list

(** Split a splat around the first occurrence of the given spread variable. *)
val partition_at_tyvar :
  Typing_shape_normalize.Row.t -> Tvid.t -> (locl_ty list * locl_ty list) option

(** Substitute solved spread variables and drop unsolved ones as empty rows. *)
val solve_spread_vars :
  env ->
  Typing_reason.t ->
  Typing_shape_normalize.Row.t ->
  env * Typing_shape_normalize.Row.t

(* == Exported only so the tests can reach them =============================
 *
 * Nothing outside typingCornersTest uses anything below. They are the pieces
 * whose behaviour the tests check directly rather than through a subtyping
 * decision, which is the whole reason this module was split out of
 * Typing_subtype: reached only through [is_sub_type], most of their
 * configurations cannot be produced at all. *)

module For_test : sig
  (** Shape interpretations of an upper bound; unconstrained means non-shape. *)
  type upper_bound_view =
    | Upper_shapes of Typing_shape_normalize.Row.t list
    | Upper_bottom
    | Upper_unconstrained

  (** Shape interpretations of a lower bound, defaulting to the bottom row. *)
  type lower_bound_view =
    | Lower_shapes of Typing_shape_normalize.Row.t list
    | Lower_bottom

  (** Whether rightward spreads prevent an element from affecting a label. *)
  module Masking : sig
    (** Definite masking, definite visibility, or a bounds-dependent result. *)
    type t =
      | Masked
      | Unmasked
      | Unknown

    (** Determine how [key] is masked in a row under the current assignment. *)
    val of_row :
      env ->
      Typing_shape_normalize.Row.t ->
      TShapeField.t option ->
      locl_ty ->
      Assignment.t ->
      Typing_reason.t ->
      t
  end

  (** Everything reachable by following bounds from the given elements. *)
  val closure : env -> Splat_elem.Set.t -> Typing_reason.t -> Splat_elem.Set.t

  (** Which corners to check for [key] at one label. Fewer than all of them when
    the element can only affect one side of the comparison. *)
  val corners_for :
    env ->
    depended_on:Splat_elem.Set.t ->
    live_sub:Splat_elem.Set.t ->
    live_super:Splat_elem.Set.t ->
    sub:Typing_shape_normalize.Row.t ->
    super:Typing_shape_normalize.Row.t ->
    TShapeField.t option ->
    locl_ty ->
    Assignment.t ->
    Typing_reason.t ->
    env * Field.Corners.t

  (** Interpret a spread element's upper bound as normalized shape rows. *)
  val bound_shape_upper :
    env -> locl_ty -> Assignment.t -> Typing_reason.t -> env * upper_bound_view

  (** Interpret a spread element's lower bound as normalized shape rows. *)
  val bound_shape_lower :
    env -> locl_ty -> Assignment.t -> Typing_reason.t -> env * lower_bound_view

  (** Project a spread element's lower and upper field bounds at one label. *)
  val field_bounds :
    env ->
    locl_ty ->
    TShapeField.t option ->
    Assignment.t ->
    Typing_reason.t ->
    env * locl_phase shape_field_type * locl_phase shape_field_type

  (** Spread elements mentioned by the given element's upper bound. *)
  val type_params_in_upper_bound :
    env -> locl_ty -> Typing_reason.t -> locl_ty list

  (** Spread elements mentioned by the given element's lower bound. *)
  val type_params_in_lower_bound :
    env -> locl_ty -> Typing_reason.t -> locl_ty list

  (** Spread elements mentioned by either bound of the given element. *)
  val type_params_in_bounds : env -> locl_ty -> Typing_reason.t -> locl_ty list
end

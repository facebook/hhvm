(* (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary. *)

open Typing_defs

type merge_result =
  | Full of locl_phase ty * bool
  | Empty_shape of bool
      (** The merge collapsed to the empty closed shape [shape()]. The merge site
          has no [Reason.t] to build the type, so the caller constructs it. The
          [bool] is the supportdyn flag. *)
  | Partial of locl_phase ty list * bool

val merge_field_descs :
  fd_left:locl_phase shape_field_type ->
  fd_right:locl_phase shape_field_type ->
  Typing_env_types.env ->
  Typing_env_types.env * locl_phase shape_field_type

val merge_shapes_simple :
  shape_left:locl_phase shape_type_simple ->
  shape_right:locl_phase shape_type_simple ->
  Typing_env_types.env ->
  Typing_env_types.env * locl_phase shape_type_simple

val merge :
  on_error:Typing_error.Reasons_callback.t option ->
  locl_phase ty list ->
  Typing_env_types.env ->
  Typing_env_types.env * Typing_error.t option * merge_result

type normalize_result =
  | Normalized_shape of locl_phase shape_type
  | Normalized_bottom
      (** The merge collapsed to the bottom row [nothing]: the row is
          uninhabited. *)

module Row : sig
  (** A normalized local shape row, including the uninhabited bottom row.

      A single operand is lifted to a simple shape, type parameter, inference
      variable, or newtype case. A multi-element splat contains only those
      operands, contains at least one opaque operand, and never contains
      adjacent shape fragments. *)
  type t

  module Opaque : sig
    type t

    type view =
      | Type_parameter of locl_phase ty
      | Type_variable of locl_phase ty
      | Newtype of locl_phase ty

    val view : t -> view

    val ty : t -> locl_phase ty
  end

  type simple = Reason.t * locl_phase shape_type_simple

  module Element : sig
    type t

    type view =
      | Shape of simple
      | Opaque of Opaque.t

    val view : t -> view

    val ty : t -> locl_phase ty
  end

  val is_bottom : t -> bool

  val as_simple : t -> locl_phase shape_type_simple option

  val of_simple : locl_phase shape_type_simple -> t

  (** Eliminate a row without exposing its normalized representation.
      [elements] receives the spread-position sequence for every inhabited row
      that is not a simple shape. *)
  val fold :
    t ->
    bottom:(unit -> 'a) ->
    simple:(locl_phase shape_type_simple -> 'a) ->
    elements:(Element.t list -> 'a) ->
    'a

  val normalize :
    on_error:Typing_error.Reasons_callback.t option ->
    Reason.t ->
    locl_phase shape_type ->
    Typing_env_types.env ->
    Typing_env_types.env * Typing_error.t option * t

  val to_ty : reason:Reason.t -> t -> locl_phase ty
end

val normalize_shape_type :
  on_error:Typing_error.Reasons_callback.t option ->
  Reason.t ->
  locl_phase shape_type ->
  Typing_env_types.env ->
  Typing_env_types.env * Typing_error.t option * normalize_result

(** The type denoted by a merge result, in normal form. A lone element IS the
    splat, so it is lifted out rather than left wrapped. *)
val ty_of_merge_result : reason:Reason.t -> merge_result -> locl_phase ty * bool

(** Canonical constructor: normalize [elems] into a shape type in normal form.
    Operations that rewrite a row must build their result with this rather than
    assembling a [Shape_splat] by hand. *)
val splat :
  on_error:Typing_error.Reasons_callback.t option ->
  reason:Reason.t ->
  locl_phase ty list ->
  Typing_env_types.env ->
  Typing_env_types.env * Typing_error.t option * locl_phase ty

(** Set [field] on a shape splat under rightmost-wins semantics, returning the
    rewritten row in normal form. *)
val set_rightmost_field :
  locl_phase ty list ->
  tshape_field_name ->
  locl_phase shape_field_type ->
  pessimize_existing:bool ->
  reason:Reason.t ->
  Typing_env_types.env ->
  Typing_env_types.env * Typing_error.t option * locl_phase ty

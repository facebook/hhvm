(* (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary. *)

open Typing_defs

type merge_result =
  | Full of locl_phase ty * bool
  | Empty_shape of bool
      (** The merge collapsed to the empty closed shape [shape()]. The merge site
          has no [Reason.t] to build the type, so the caller constructs it. The
          [bool] is the supportdyn flag. *)
  | Partial of locl_phase ty list * bool

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

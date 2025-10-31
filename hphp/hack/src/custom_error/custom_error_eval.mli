(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Invalid_pattern of string * Validation_err.t list

module Value : sig
  type t =
    | Ty of Typing_defs_core.locl_ty
    | Name of (Pos_or_decl.t * string)
    | File of Relative_path.t
    | Member_name of string
  [@@deriving compare, sexp]
end

val eval_typing_error :
  Custom_error_config.t ->
  err:Typing_error.t ->
  (string, Value.t) Base.Either.t list list

val eval_naming_error :
  Custom_error_config.t ->
  err:Naming_error.t ->
  (string, Value.t) Base.Either.t list list

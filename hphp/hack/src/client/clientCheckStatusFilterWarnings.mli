(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Code = Error_codes.Warning

type switch =
  | WAll
  | WNone
  | Code_on of Code.t
  | Code_off of Code.t
  | Ignored_files of Str.regexp

val filter :
  switch list -> Errors.finalized_error list -> Errors.finalized_error list

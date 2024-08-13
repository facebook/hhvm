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

module Filter : sig
  type t

  (** [default_all] controls wether an empty list of switches is equivalent to [WAll]
    as opposed to [WNone] *)
  val make : default_all:bool -> switch list -> t
end

val filter :
  Filter.t -> Errors.finalized_error list -> Errors.finalized_error list

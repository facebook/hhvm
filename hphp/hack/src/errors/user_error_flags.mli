(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  stripped_existential: bool;
      (** Tracks whether existential (or 'expression dependent') types were stripped in the error message *)
}
[@@deriving eq, hash, ord, show]

val create : ?stripped_existential:bool -> unit -> t

val empty : t

val to_json : t -> Hh_json.json

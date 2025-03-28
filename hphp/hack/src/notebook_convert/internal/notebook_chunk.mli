(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(** Internal representation for converting between notebook ipynb format (.json) and Hack format (.php) *)

module Id : sig
  type t [@@deriving ord]

  val zero : t

  val next : t -> t
end

type chunk_kind =
  | Hack
  | Non_hack of { cell_type: string }
[@@deriving eq, ord]

type t = {
  id: Id.t;
  chunk_kind: chunk_kind;
  contents: string;
  cell_bento_metadata: Hh_json.json option;
      (** Corresponds to the cell-level "metadata" field in ipynb format.  *)
}
[@@deriving ord]

val is_chunk_start_comment : string -> bool

val is_chunk_end_comment : string -> bool

(** Prepend a magic comment that preserves information about cells. This will enable us to
reconstruct the notebook from the generated Hack (with some loss of position information).
*)
val to_hack : t -> string

val of_hack : comment:string -> string -> (t, Notebook_convert_error.t) result

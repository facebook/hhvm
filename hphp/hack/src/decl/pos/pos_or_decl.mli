(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Positions in the decl heap are compressed and need to be resolved
    before being used or printed. This type represent any position, either
    a fully resolved position or a decl position.
    TODO (catg) for now, it is transparantly equal to Pos.t, but its implementation
    will change in the future. *)
type t = Pos.t [@@deriving eq, ord, show]

val none : t

(** Fill in the gap "between" first position and second position.
    Not valid if from different files or second position precedes first *)
val btw : t -> t -> t

(** Essentially an upcast. *)
val of_raw_pos : Pos.t -> t

(** Compress a position to be stored in the decl heap. *)
val make_decl_pos : Pos.t -> Decl_reference.t -> t

val is_hhi : t -> bool

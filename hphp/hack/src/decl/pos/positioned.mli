(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Compress the position of a positioned value to be stored in the decl heap. *)
val make_for_decl : Pos.t * 'a -> Decl_reference.t -> Pos_or_decl.t * 'a

(** Compress the position of a positioned value to be stored in the decl heap.
    If no decl reference is given, the position is not compressed but
    simply converted. *)
val make_for_decl_of_option :
  Pos.t * 'a -> Decl_reference.t option -> Pos_or_decl.t * 'a

(** This may become unsafe in the future as we change the implementation
    of positions in the decl heap. Avoid using in new code.
    Use an id from an AST instead of from a decl or type,
    or resolve decl position to a raw position using a provider context. *)
val unsafe_to_raw_positioned : Pos_or_decl.t * 'a -> Pos.t * 'a

(** This is essentially an upcast. *)
val of_raw_positioned : Pos.t * 'a -> Pos_or_decl.t * 'a

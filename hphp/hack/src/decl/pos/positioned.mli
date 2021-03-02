(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Compress the position of a positioned value to be stored in the decl heap. *)
val make_for_decl : Pos.t * 'a -> Decl_reference.t -> Pos_or_decl.t * 'a

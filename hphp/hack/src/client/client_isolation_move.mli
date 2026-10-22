(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Dir_move : sig
  (** A directory moved on disk, which can be put back. *)
  type t

  (** [move ~source ~target] renames the directory [source] to [target], both
      absolute, and makes the parents of [target] that are missing. [target] must
      not exist. When it fails it removes the parents it made, and [Error] says
      why, and also what it could not remove. *)
  val move : source:string -> target:string -> (t, string) result

  (** [undo t] renames the directory back and removes the directories [move]
      made, except any that is no longer empty. A step that is already done is
      skipped, so undoing twice is harmless. Raises if it cannot undo the move
      completely, as when something other than an empty directory is where the
      directory was. *)
  val undo : t -> unit
end

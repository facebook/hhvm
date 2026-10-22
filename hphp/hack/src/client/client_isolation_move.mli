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

module Move_error : sig
  (** A diagnostic with severity error, in the file it was reported in. *)
  type t = {
    code: int;
    file: Relative_path.t;
    line: int;
    message: string;
  }

  (** The errors in the server's status. Warnings are left out. *)
  val of_status : Server_command_types.Server_status.t -> t list

  (** [introduced ~source ~target ~baseline ~after] is what is in [after] and not
      in [baseline], where [after] was reported once the directory [source] had
      moved to [target]. It is named as it was before the move: a file under
      [target] as the file under [source], and [target] in a message as [source]. *)
  val introduced :
    source:Relative_path.t ->
    target:Relative_path.t ->
    baseline:t list ->
    after:t list ->
    t list

  (** Whether the two have the same errors, in any order. *)
  val same : t list -> t list -> bool
end

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Option.Monad_infix

module ClientId : sig
  type t = int

  val make : unit -> t
end = struct
  type t = int

  let next_id : t ref = ref 0

  let make () =
    let id = !next_id in
    next_id := id + 1;
    id
end

type client_id = ClientId.t

type t = {
  id: ClientId.t;
  client: ClientProvider.client;
  open_files: Relative_path.Set.t;
}

let ide_info : t option ref = ref None

let make_persistent_and_track_new_ide client =
  let client = ClientProvider.make_persistent client in
  let id = ClientId.make () in
  Hh_logger.info "[Ide_info_store] New tracked IDE with ID %d." id;
  ide_info := Some { id; client; open_files = Relative_path.Set.empty };
  client

let ide_disconnect () =
  Option.iter !ide_info ~f:(fun ide_info ->
      Hh_logger.info "[Ide_info_store] IDE with ID %d disconnected" ide_info.id);
  ide_info := None

let with_open_file file ide =
  { ide with open_files = Relative_path.Set.add ide.open_files file }

let open_file file = ide_info := !ide_info >>| with_open_file file

let with_close_file file ide =
  { ide with open_files = Relative_path.Set.remove ide.open_files file }

let close_file file = ide_info := !ide_info >>| with_close_file file

let get () = !ide_info

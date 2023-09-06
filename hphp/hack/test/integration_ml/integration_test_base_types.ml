(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

type 'a new_client_type = RequestResponse of 'a ServerCommandTypes.t

type disk_changes_type = (string * string) list

type 'a loop_inputs = {
  disk_changes: disk_changes_type;
  new_client: 'a new_client_type option;
}

type 'a loop_outputs = {
  did_read_disk_changes: bool;
  total_changed_files_count: int;
  total_rechecked_count: int;
  last_actual_total_rechecked_count: int option;
  new_client_response: 'a option;
  push_messages: ServerCommandTypes.push list;
}

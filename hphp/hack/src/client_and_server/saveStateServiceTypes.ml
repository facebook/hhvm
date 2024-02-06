(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** An alias for the errors type that we marshal to and unmarshal from the saved state.
Keep this in sync with saved_states.rs:dump_error_files *)
type saved_state_errors = Relative_path.Set.t

type save_state_result = { dep_table_edges_added: int }

type save_naming_result = {
  nt_files_added: int;
  nt_symbols_added: int;
}

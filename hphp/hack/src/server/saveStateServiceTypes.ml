(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** An alias for the errors type that we marshal to and unmarshal from the saved state.
Although this is represented as a list, no entries in the list share the same phase.
(It might have been nicer to store it as a phase map). *)
type saved_state_errors = (Errors.phase * Relative_path.Set.t) list

type save_state_result = { dep_table_edges_added: int }

type save_naming_result = {
  nt_files_added: int;
  nt_symbols_added: int;
}

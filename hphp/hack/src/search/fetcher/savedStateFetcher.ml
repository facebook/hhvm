(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Implement this stub to select a location where files are stored *)
let get_filename_for_symbol_index (_extension: string): string =
  raise Not_found

(* Implement this stub to save time by fetching symbol indexes *)
let find_saved_symbolindex (): (string, string) result =
  Error "Not implemented"

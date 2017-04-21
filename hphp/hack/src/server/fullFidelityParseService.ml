(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Entry Point *)
let go filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = Full_fidelity_source_text.from_file file in
  let syntax_tree = Full_fidelity_syntax_tree.make source_text in
  let json = Full_fidelity_syntax_tree.to_json syntax_tree in
  Hh_json.json_to_string json

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Bad_type_logger_types

let should_log log_level log_mask = log_level land log_mask_to_enum log_mask > 0

let string_of_pos pos =
  let path = Pos.filename pos |> Relative_path.suffix in
  let filename =
    match String.rsplit2 ~on:'/' path with
    | Some (_, filename) -> filename
    | None -> path
  in
  let (line, col) = Pos.line_column pos in
  Printf.sprintf "%s:%d:%d" filename line (col + 1)

let string_of_context_id = function
  | Function fx_name -> fx_name
  | Method (class_name, method_name) -> class_name ^ "::" ^ method_name

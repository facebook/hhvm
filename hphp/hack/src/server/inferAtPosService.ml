(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_message

(* The first string is the pretty-printed type, the second is the JSON *)
type result = (string * string) option

let infer_result_to_ide_response typename =
  match typename with
  | None ->
    Infer_type_response { type_string = None; type_json = None }
  | Some (str, _json) ->
    Infer_type_response { type_string = Some str; type_json = None }

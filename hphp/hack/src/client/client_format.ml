(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let to_json result =
  let (error, result, internal_error) =
    match result with
    | Ok s -> ("", s, false)
    | Error s -> (s, "", true)
  in
  `Assoc
    [
      ("error_message", `String error);
      ("result", `String result);
      ("internal_error", `Bool internal_error);
    ]

let print_json res = print_endline (Yojson.Safe.to_string (to_json res))

let print_readable = function
  | Ok res -> print_string res
  | _ -> ()

let go res output_json =
  if output_json then
    print_json res
  else
    print_readable res

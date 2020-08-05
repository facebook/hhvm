(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go (results : ServerLintTypes.result) output_json error_format =
  let (results, highlighted_error_format) = results in
  let error_format =
    match error_format with
    | None ->
      if highlighted_error_format then
        Errors.Highlighted
      else
        Errors.Context
    | Some ef -> ef
  in
  if output_json then
    ServerLint.output_json stdout results
  else
    ServerLint.output_text stdout results error_format

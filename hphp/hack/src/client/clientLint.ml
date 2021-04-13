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
  if output_json then
    ServerLintTypes.output_json stdout results
  else
    ServerLintTypes.output_text stdout results error_format

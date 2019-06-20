(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Env = Typing_env
module Reason = Typing_reason

let intersect_list env r tyl =
  (* TODO T44713456 dummy implementation for now. *)
  env, (r, Tintersection tyl)

(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv

let go : ServerEnv.env -> Errors.finalized_error list =
 fun env ->
  let error_list = Errors.get_sorted_error_list env.errorl in
  let error_list = List.map ~f:Errors.to_absolute error_list in
  error_list

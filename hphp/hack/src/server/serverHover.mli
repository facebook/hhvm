(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns detailed information about the symbol or expression at the given
    location. *)
val go_ctx :
  ctx:ServerIdeContext.t ->
  entry:ServerIdeContext.entry ->
  line:int ->
  char:int ->
  basic_only:bool ->
  HoverService.result

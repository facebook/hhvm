(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val ai_check :
  ServerEnv.genv ->
  Naming_table.t ->
  ServerEnv.env ->
  float ->
  ServerEnv.env * float

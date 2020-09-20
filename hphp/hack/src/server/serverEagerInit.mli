(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerInitTypes

val init :
  ServerEnv.genv -> lazy_level -> ServerEnv.env -> ServerEnv.env * float

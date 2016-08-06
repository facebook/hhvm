(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type result = Pos.absolute list

val go_from_file: (string * int * int) -> ServerEnv.env -> result

val go: (string * int * int) -> TypecheckerOptions.t -> result

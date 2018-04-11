(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns the docblock for the symbol or expression at the given location. *)
val go : ServerEnv.env -> (string * int * int) -> DocblockService.result

(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* mode -> filename -> (output if changed * list of errors *)
val convert : Convert_ty.mode -> string -> string option * string list

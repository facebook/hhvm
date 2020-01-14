(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_local_changes : unit -> unit

val revert_local_changes : unit -> unit

(* When typechecking a content buffer in IDE mode,
* this is the path that will be assigned to it *)
val path : Relative_path.t

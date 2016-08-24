(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

 type patch =
  | Insert of insert_patch
  | Remove of Pos.absolute
  | Replace of insert_patch

and insert_patch = {
  pos: Pos.absolute;
  text: string;
}

type action =
  | ClassRename of string * string (* old_name * new_name *)
  | MethodRename of string * string * string
    (* class_name * old_name * new_name*)
  | FunctionRename of string * string (* old_name * new_name *)

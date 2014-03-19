(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * Checks if x is a www directory by looking for ".hhconfig".
 *)
let is_www_directory (path : string) : bool =
   let arcconfig = Filename.concat path ".hhconfig" in
   Sys.file_exists arcconfig

let find_www_root_from_absolute_path (path : string): string =
  let rec aux subdir =
    if is_www_directory subdir
    then subdir
    else let parent = Filename.dirname subdir in
    if parent = "/"
    then raise Not_found
    else aux parent
  in aux path


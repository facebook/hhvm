(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val is_www_directory : ?config:string -> Path.t -> bool

val assert_www_directory : ?config:string -> Path.t -> unit

(** Our command-line tools generally take a "root" parameter, and if none is
supplied then they fall back to a search based on the current directory.
This code implements that -- it ensures that either 0 or 1 root parameters
were passed (and prints to stderr and exits with code 1 if there were more);
if there were 0 parameters then it walks from CWD to find .hhconfig
and if there was 1 then it walks from that to find .hhconfig.
Then it validates that .hhconfig really is there, and prints to stderr
and exits with code 1 if it wasn't.

CARE! Don't call this from arbitrary code in client or server, passing in
None in the hope that you'll pick up the root directory for your current process.
Doing so will mean you ignore the "root" parameter at the command-line! *)
val interpret_command_line_root_parameter :
  ?config:string -> string list -> Path.t

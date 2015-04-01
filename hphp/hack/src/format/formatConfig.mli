(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  indent_offset        : int  ;
  line_width           : int  ;
  indent_case          : bool ;
  align_cascaded_calls : bool ;
}

val load : Relative_path.t -> t

val default_config : t

val indent_offset : t -> int
val line_width : t -> int
val indent_case : t -> bool
val align_cascaded_calls : t -> bool

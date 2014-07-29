(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let go (fn, line, char) oc =
  let clean () =
    Typing_defs.infer_type := None;
    Typing_defs.infer_target := None;
    Typing_defs.infer_pos := None;
  in
  clean ();
  Typing_defs.infer_target := Some (line, char);
  ServerIdeUtils.check_file_input fn;
  let pos = !Typing_defs.infer_pos in
  let ty = !Typing_defs.infer_type in
  clean ();
  Marshal.to_channel oc (pos, ty) [];
  flush oc

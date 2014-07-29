(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let go genv env oc content line char =
  ArgumentInfoService.attach_hooks (line, char);
  let funs, classes = ServerIdeUtils.declare content in
  ServerIdeUtils.fix_file_and_def content;
  let pos, expected =
    match ArgumentInfoService.get_result() with
    | Some (pos, expected) -> pos, expected
    | _ ->(-1), []
  in
  ArgumentInfoService.detach_hooks();
  ServerIdeUtils.revive funs classes;
  Marshal.to_channel oc (pos, expected) [];
  flush oc

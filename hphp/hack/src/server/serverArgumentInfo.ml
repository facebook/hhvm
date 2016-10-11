(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type result = int * (string option * string) list

let go content line char tcopt =
  ArgumentInfoService.attach_hooks (line, char);
  let pos, expected =
    ServerIdeUtils.declare_and_check content ~f:begin fun _ _ ->
      match ArgumentInfoService.get_result() with
      | Some (pos, expected) -> pos, expected
      | _ ->(-1), []
    end tcopt
  in
  ArgumentInfoService.detach_hooks();
  pos, expected

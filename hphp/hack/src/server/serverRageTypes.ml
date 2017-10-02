(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* To make the rage output more useful, it's broken into rageItems. Each      *)
(* can optionally be tagged with a title like "/repo/file.php:modified_hh" in *)
(* case we'll want to say download that item in isolation.                    *)
type rageItem = {
  title: string option;
  data: string;
}

type result = rageItem list

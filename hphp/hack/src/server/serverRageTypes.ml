(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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

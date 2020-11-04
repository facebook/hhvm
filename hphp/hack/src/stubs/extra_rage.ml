(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let create_repro ~mergebase ~patch_script =
  ignore (mergebase, patch_script);
  Lwt.return_none

let ooms () = Lwt.return ""

let shell () = Lwt.return ""

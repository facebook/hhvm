(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let clowder_download ?(timeout : float option) (handle : string) =
  ignore (timeout, handle);
  failwith "not implemented in this build"

let clowder_upload_and_get_handle ?(timeout : float option) (input : string) =
  ignore (timeout, input);
  failwith "not implemented in this build"

let clowder_upload_and_get_url ?(timeout : float option) (input : string) =
  ignore (timeout, input);
  failwith "not implemented in this build"

let clowder_upload_and_get_shellscript
    ?(timeout : float option) (input : string) =
  ignore (timeout, input);
  failwith "not implemented in this build"

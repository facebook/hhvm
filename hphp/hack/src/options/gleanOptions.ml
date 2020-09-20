(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = GlobalOptions.t [@@deriving show]

let service = GlobalOptions.glean_service

let hostname = GlobalOptions.glean_hostname

let port = GlobalOptions.glean_port

let reponame = GlobalOptions.glean_reponame

let default = GlobalOptions.default

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

type t = GlobalOptions.t [@@deriving show]

let service t = t.GlobalOptions.glean_service

let hostname t = t.GlobalOptions.glean_hostname

let port t = t.GlobalOptions.glean_port

let reponame t = t.GlobalOptions.glean_reponame

let default = GlobalOptions.default

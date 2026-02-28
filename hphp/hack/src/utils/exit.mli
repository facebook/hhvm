(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val exit :
  ?msg:string -> ?telemetry:Telemetry.t -> ?stack:string -> Exit_status.t -> 'a

val add_hook_upon_clean_exit : (Exit_status.finale_data -> unit) -> unit

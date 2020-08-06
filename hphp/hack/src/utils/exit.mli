(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type finale_data = {
  exit_status: Exit_status.t;
  msg: string;
  stack: Utils.callstack;
}

val exit : ?msg:string -> ?stack:string -> Exit_status.t -> 'a

val set_finale_file_for_eventual_exit : string -> unit

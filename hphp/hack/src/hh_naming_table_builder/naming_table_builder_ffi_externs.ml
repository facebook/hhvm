(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type build_progress

external build :
  www:string -> custom_hhi_path:string -> output:string -> build_progress
  = "naming_table_builder_ffi_build"

(** The naming table build represents failure in one of two ways: by
   throwing an OCaml exception (we unwrap the result and obtain a call
   stack), or by returning a non-zero exit status. The exit statuses are
   described by `naming_table_builder::ExitStatus`. All other failures are
   represented by exceptions. *)
external poll_exn : build_progress -> int option
  = "naming_table_builder_ffi_poll"

let build ~(www : Path.t) ~(custom_hhi_path : Path.t) ~(output : Path.t) :
    build_progress =
  build
    ~www:(Path.to_string www)
    ~custom_hhi_path:(Path.to_string custom_hhi_path)
    ~output:(Path.to_string output)

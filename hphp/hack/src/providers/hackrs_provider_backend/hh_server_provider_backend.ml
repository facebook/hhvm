(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external register : unit -> unit = "hh_server_provider_backend_register"

let () = register ()

external make_ffi :
  root:string ->
  hhi_root:string ->
  tmp:string ->
  ParserOptions.t ->
  Rust_provider_backend.t = "hh_server_provider_backend_make"

let make popt =
  make_ffi
    ~root:Relative_path.(path_of_prefix Root)
    ~hhi_root:Relative_path.(path_of_prefix Hhi)
    ~tmp:Relative_path.(path_of_prefix Tmp)
    popt

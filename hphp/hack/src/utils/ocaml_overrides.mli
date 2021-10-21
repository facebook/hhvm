(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Ocaml_unix = Unix
module Ocaml_Sys = Sys

module Unix : sig
  (* Import ocaml's Unix module signature *)
  include module type of Ocaml_unix
end

module Sys : sig
  (* Import ocaml's Sys module signature *)
  include module type of Ocaml_Sys

  (* Shadow the ones we redeclare cause they all have `external` in the
   * original signature.
   *)
  val getcwd : unit -> string

  val chdir : string -> unit

  val is_directory : string -> bool

  val rename : string -> string -> unit

  val file_exists : string -> bool

  val readdir : string -> string array
end

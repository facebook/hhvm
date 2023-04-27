(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = private {
  mutable is_generated: bool;
  mutable fully_generated: bool;
  mutable source: string option;
  mutable command: string option;
  mutable class_: string option;
  mutable signature: string option;
}

(* one linear pass of the string to extract the gencode info *)
val get_gencode_status : string -> t

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  implicit_inherit_sdt: bool;
  everything_sdt: bool;
  enable_strict_const_semantics: int;
  class_class_type: bool;
  safe_abstract: bool;
}
[@@deriving show]

val from_global_options : GlobalOptions.t -> t

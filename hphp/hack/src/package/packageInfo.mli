(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t [@@deriving show, eq]

val empty : t

val from_packages : Package.t list -> t

val get_package_for_module : t -> string -> Package.t option

val get_package : t -> string -> Package.t option

val package_exists : t -> string -> bool

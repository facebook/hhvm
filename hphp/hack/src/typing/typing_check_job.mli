(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_fun :
  Provider_context.t -> Relative_path.t -> string -> Tast.def list option

val type_class :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

val check_typedef :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

val check_const :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

val check_module :
  Provider_context.t -> Relative_path.t -> string -> Tast.def option

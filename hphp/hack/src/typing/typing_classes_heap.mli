(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Classes : sig
  type key = StringKey.t
  type t

  val get : key -> t option
  val mem : key -> bool
  val find_unsafe : key -> t
end

type t = Classes.t

val need_init             : t -> bool
val members_fully_known   : t -> bool
val abstract              : t -> bool
val final                 : t -> bool
val const                 : t -> bool
val deferred_init_members : t -> SSet.t
val kind                  : t -> Ast.class_kind
val is_xhp                : t -> bool
val is_disposable         : t -> bool
val name                  : t -> string
val pos                   : t -> Pos.t
val tparams               : t -> decl tparam list
val consts                : t -> class_const SMap.t
val typeconsts            : t -> typeconst_type SMap.t
val props                 : t -> class_elt SMap.t
val sprops                : t -> class_elt SMap.t
val methods               : t -> class_elt SMap.t
val smethods              : t -> class_elt SMap.t
val construct             : t -> class_elt option * bool
val ancestors             : t -> decl ty SMap.t
val req_ancestors         : t -> requirement list
val req_ancestors_extends : t -> SSet.t
val extends               : t -> SSet.t
val enum_type             : t -> enum_type option
val decl_errors           : t -> Errors.t option

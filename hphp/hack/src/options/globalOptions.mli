(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t
val make :
  bool -> bool -> SSet.t option -> SSet.t -> (string * string) list -> t
val tco_assume_php : t -> bool
val tco_unsafe_xhp : t -> bool
val tco_user_attrs : t -> SSet.t option
val tco_experimental_feature_enabled : t -> SSet.elt -> bool
val tco_allowed_attribute : t -> SSet.elt -> bool
val po_auto_namespace_map : t -> (string * string) list
val default : t
val permissive : t
val tco_experimental_dict : string
val tco_experimental_instanceof : string
val tco_experimental_all : SSet.t

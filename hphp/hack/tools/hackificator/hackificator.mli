(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val hackify: 
  upgrade:bool -> string -> Common.filename -> string option

val hackify_thrift: 
  Common.filename -> string option

val split_vars: 
  Common.filename -> string option

val hackify_class_of_typed_interface:
  Common.filename -> Common.filename -> string

val with_trying_modif:
  ?undo_when_error:bool ->
  www:string -> 
  Common.filename -> (Common.filename -> string option) list -> bool

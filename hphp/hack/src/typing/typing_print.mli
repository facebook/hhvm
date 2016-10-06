(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

val error: 'a Typing_defs.ty_ -> string
val suggest: 'a Typing_defs.ty -> string
val full: Typing_env.env -> 'a Typing_defs.ty -> string
val full_rec: Typing_env.env -> int -> 'a Typing_defs.ty -> string
val full_strip_ns: Typing_env.env -> 'a Typing_defs.ty -> string
val debug: Typing_env.env -> 'a Typing_defs.ty -> string
val class_: TypecheckerOptions.t -> Typing_defs.class_type -> string
val gconst: TypecheckerOptions.t -> Decl_heap.GConst.t -> string
val fun_: TypecheckerOptions.t -> Decl_heap.Fun.t -> string
val typedef: TypecheckerOptions.t -> Decl_heap.Typedef.t -> string

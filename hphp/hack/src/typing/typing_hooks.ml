(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let (id_hooks: (Pos.t * string -> unit) list ref) = ref []

let (smethod_hooks: (Typing_defs.class_type -> Pos.t * string ->
                     Typing_env.env -> Nast.class_id option ->
                     unit) list ref) = ref []

let (cmethod_hooks: (Typing_defs.class_type -> Pos.t * string ->
                     Typing_env.env -> Nast.class_id option ->
                     unit) list ref) = ref []

let attach_smethod_hook hook =
  smethod_hooks := hook :: !smethod_hooks

let attach_cmethod_hook hook =
  cmethod_hooks := hook :: !cmethod_hooks

let attach_id_hook hook =
  id_hooks := hook :: !id_hooks

let dispatch_id_hook id =
  List.iter begin fun hook -> hook id end !id_hooks

let dispatch_smethod_hook class_ id env cid =
  List.iter begin fun hook -> hook class_ id env cid end !smethod_hooks

let dispatch_cmethod_hook class_ id env cid =
  List.iter begin fun hook -> hook class_ id env cid end !cmethod_hooks

let remove_all_hooks () =
  id_hooks := [];
  cmethod_hooks := [];
  smethod_hooks := [];

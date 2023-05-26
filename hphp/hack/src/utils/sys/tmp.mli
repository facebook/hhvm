(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(** [temp_dir parent_dir prefix ] creates a new temporary directory under parent_dir.
    The name of that directory is made of the given prefix followed by a random integer. *)
val temp_dir : string -> string -> string

val hh_server_tmp_dir : string

(** Create subdirectory in [hh_server_tmp_dir] as [description_what_for]/zSpathzStozS[root] *)
val make_dir_in_tmp : description_what_for:string -> root:Path.t -> string

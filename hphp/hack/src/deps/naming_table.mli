(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t
type fast = FileInfo.names Relative_path.Map.t
type saved_state_info = FileInfo.saved Relative_path.Map.t

(* Querying and updating naming tables. *)
val combine : t -> t -> t
val create : FileInfo.t Relative_path.Map.t -> t
val empty : t
val filter : t -> f:(Relative_path.t -> FileInfo.t -> bool) -> t
val fold : t -> init:'b -> f:(Relative_path.t -> FileInfo.t -> 'b -> 'b) -> 'b
val get_files : t -> Relative_path.t list
val get_file_info : t -> Relative_path.t -> FileInfo.t option
val get_file_info_unsafe : t -> Relative_path.t -> FileInfo.t
val has_file : t -> Relative_path.t -> bool
val iter : t -> f:(Relative_path.t -> FileInfo.t -> unit) -> unit
val update : t -> Relative_path.t -> FileInfo.t  -> t

(* Converting between different types of naming tables. *)
val from_saved : saved_state_info -> t
val to_saved : t -> saved_state_info
val to_fast: t -> fast
val saved_to_fast: saved_state_info -> fast

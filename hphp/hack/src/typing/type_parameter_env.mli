(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type tparam_bounds = Typing_set.t

type tparam_info = Typing_kinding_defs.kind

type t

val empty : t

val mem : string -> t -> bool

val get : string -> t -> tparam_info option

val get_with_pos : string -> t -> (Pos_or_decl.t * tparam_info) option

val add : def_pos:Pos_or_decl.t -> string -> tparam_info -> t -> t

val union : t -> t -> t

val size : t -> int

val fold : (string -> tparam_info -> 'a -> 'a) -> t -> 'a -> 'a

val merge_env :
  'env ->
  t ->
  t ->
  combine:
    ('env ->
    string ->
    (Pos_or_decl.t * tparam_info) option ->
    (Pos_or_decl.t * tparam_info) option ->
    'env * (Pos_or_decl.t * tparam_info) option) ->
  'env * t

val get_lower_bounds : t -> string -> Typing_defs.locl_ty list -> tparam_bounds

val get_upper_bounds : t -> string -> Typing_defs.locl_ty list -> tparam_bounds

(* value > 0 indicates higher-kinded type parameter *)
val get_arity : t -> string -> int

val get_reified : t -> string -> Aast.reify_kind

val get_enforceable : t -> string -> bool

val get_newable : t -> string -> bool

val get_names : t -> string list

val is_consistent : t -> bool

val mark_inconsistent : t -> t

val add_upper_bound :
  ?intersect:
    (Typing_defs.locl_ty ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list) ->
  t ->
  string ->
  Typing_defs.locl_ty ->
  t

val add_lower_bound :
  ?union:
    (Typing_defs.locl_ty ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list) ->
  t ->
  string ->
  Typing_defs.locl_ty ->
  t

val add_generic_parameters : t -> Typing_defs.decl_tparam list -> t

val remove_lower_bound : t -> string -> Typing_defs.locl_ty -> t

val remove_upper_bound : t -> string -> Typing_defs.locl_ty -> t

val remove : t -> string -> t

val get_parameter_names : tparam_info -> string list

val pp : Format.formatter -> t -> unit

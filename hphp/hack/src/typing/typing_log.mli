(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val out_channel : Stdlib.out_channel ref

val log_key : string -> unit

val log_env_diff :
  Pos.t ->
  ?function_name:string ->
  Typing_env_types.env ->
  Typing_env_types.env ->
  unit

val hh_show_env : ?function_name:string -> Pos.t -> Typing_env_types.env -> unit

val hh_show_full_env : Pos.t -> Typing_env_types.env -> unit

val hh_show : Pos.t -> Typing_env_types.env -> Typing_defs.locl_ty -> unit

(** Simple type of possible log data *)
type log_structure =
  | Log_head of string * log_structure list
  | Log_type of string * Typing_defs.locl_ty
  | Log_decl_type of string * Typing_defs.decl_ty
  | Log_type_i of string * Typing_defs.internal_type

val log_with_level :
  Typing_env_types.env -> string -> level:int -> (unit -> unit) -> unit

val log_types :
  Pos_or_decl.t -> Typing_env_types.env -> log_structure list -> unit

val log_escape :
  ?level:int ->
  Pos_or_decl.t ->
  Typing_env_types.env ->
  string ->
  string list ->
  unit

val log_prop :
  int ->
  Pos_or_decl.t ->
  string ->
  Typing_env_types.env ->
  Typing_logic.subtype_prop ->
  unit

val log_tparam_instantiation :
  Typing_env_types.env -> Pos.t -> string -> Typing_defs.locl_ty -> unit

val log_new_tvar_for_new_object :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  'a * string ->
  'b Typing_defs.tparam ->
  unit

val log_new_tvar_for_tconst :
  Typing_env_types.env ->
  Pos.t * Tvid.t ->
  Typing_defs.pos_id ->
  Typing_defs.locl_ty ->
  unit

val log_new_tvar_for_tconst_access :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  string ->
  Typing_defs.pos_id ->
  unit

val log_intersection :
  level:int ->
  Typing_env_types.env ->
  Typing_reason.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  inter_ty:Typing_defs.locl_ty ->
  unit

val log_type_access :
  level:int ->
  Typing_defs.locl_ty ->
  Typing_defs.pos_id ->
  Typing_env_types.env * Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val log_localize :
  level:int ->
  Typing_defs.expand_env ->
  Typing_defs.decl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val increment_feature_count : Typing_env_types.env -> string -> unit

val log_pessimise_prop : Typing_env_types.env -> Pos.t -> string -> unit

val log_pessimise_return :
  ?level:int -> Typing_env_types.env -> Pos.t -> string option -> unit

val log_pessimise_poisoned_return :
  ?level:int -> Typing_env_types.env -> Pos.t -> string -> unit

val log_pessimise_param :
  Typing_env_types.env ->
  is_promoted_property:bool ->
  Pos.t ->
  Ast_defs.param_kind ->
  string ->
  unit

val log_sd_pass : ?level:int -> Typing_env_types.env -> Pos.t -> unit

module GlobalInference : sig
  val log_merging_subgraph : Typing_env_types.env -> Pos.t -> unit

  val log_merging_var : Typing_env_types.env -> Pos.t -> Tvid.t -> unit
end

module GI = GlobalInference

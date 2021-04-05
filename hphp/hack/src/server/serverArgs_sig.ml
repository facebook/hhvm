(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Abstract_types = struct
  type options
end

module type S = sig
  include module type of Abstract_types

  include module type of Cli_args

  type saved_state_target =
    | Informant_induced_saved_state_target of
        ServerMonitorUtils.target_saved_state
    | Saved_state_target_info of saved_state_target_info
  [@@deriving show]

  (****************************************************************************)
  (* The main entry point *)
  (****************************************************************************)

  val default_options : root:string -> options

  val default_options_with_check_mode : root:string -> options

  val parse_options : unit -> options

  (****************************************************************************)
  (* Accessors *)
  (****************************************************************************)

  val ai_mode : options -> Ai_options.t option

  val check_mode : options -> bool

  val concatenate_prefix : options -> string option

  val config : options -> (string * string) list

  val custom_telemetry_data : options -> (string * string) list

  val dump_fanout : options -> bool

  val dynamic_view : options -> bool

  val enable_ifc : options -> string list

  val from : options -> string

  val gen_saved_ignore_type_errors : options -> bool

  val ignore_hh_version : options -> bool

  val saved_state_ignore_hhconfig : options -> bool

  val json_mode : options -> bool

  val log_inference_constraints : options -> bool

  val max_procs : options -> int option

  val no_load : options -> bool

  val prechecked : options -> bool option

  val profile_log : options -> bool

  val remote : options -> bool

  val replace_state_after_saving : options -> bool

  val root : options -> Path.t

  val save_filename : options -> string option

  val save_with_spec : options -> save_state_spec_info option

  val save_64bit : options -> string option

  val save_naming_filename : options -> string option

  val should_detach : options -> bool

  val waiting_client : options -> Unix.file_descr option

  val watchman_debug_logging : options -> bool

  val with_saved_state : options -> saved_state_target option

  val is_using_precomputed_saved_state : options -> bool

  val allow_non_opt_build : options -> bool

  val write_symbol_info : options -> string option

  (****************************************************************************)
  (* Setters *)
  (****************************************************************************)

  val set_ai_mode : options -> Ai_options.t option -> options

  val set_check_mode : options -> bool -> options

  val set_gen_saved_ignore_type_errors : options -> bool -> options

  val set_max_procs : options -> int -> options

  val set_saved_state_target :
    options -> ServerMonitorUtils.target_saved_state option -> options

  val set_no_load : options -> bool -> options

  val set_config : options -> (string * string) list -> options

  (****************************************************************************)
  (* Misc *)
  (****************************************************************************)
  val print_json_version : unit -> unit

  val to_string : options -> string
end

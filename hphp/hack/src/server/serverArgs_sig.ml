module Types = struct

  (** Commandline arg "--with-mini-state" constructs this record. *)
  type saved_state_target_info = {
    changes: Relative_path.t list;
    corresponding_base_revision: string;
    deptable_fn: string;
    prechecked_changes: Relative_path.t list;
    saved_state_fn: string;
  }

  type saved_state_target =
    | Informant_induced_saved_state_target of ServerMonitorUtils.target_saved_state
    | Saved_state_target_info of saved_state_target_info

  (* The idea of a file range necessarily means that the hypothetical list
    of them is sorted in some way. It is valid to have None as either endpoint
    because that simply makes it open-ended. For example, a range of files
    { None - "/some/path" } includes all files with path less than /some/path *)
  type files_to_check_range = {
    from_prefix_incl: Relative_path.t option;
    to_prefix_excl: Relative_path.t option;
  }

  type files_to_check_spec =
  | Range of files_to_check_range
  | Prefix of Relative_path.t

  type save_state_spec_info = {
    files_to_check: files_to_check_spec list;
    (* The base name of the file into which we should save the naming table. *)
    filename: string;
    (* Indicates whether we should generate a state in the presence of errors. *)
    gen_with_errors: bool;
  }

end

module Abstract_types = struct
  type options
end

module type S = sig
  include module type of Types
  include module type of Abstract_types

  (****************************************************************************)
  (* The main entry point *)
  (****************************************************************************)

  val default_options: root:string -> options
  val parse_options: unit -> options

  (****************************************************************************)
  (* Accessors *)
  (****************************************************************************)

  val ai_mode: options -> Ai_options.t option
  val check_mode: options -> bool
  val config: options -> (string * string) list
  val dynamic_view: options -> bool
  val from: options -> string
  val gen_saved_ignore_type_errors: options -> bool
  val ignore_hh_version: options -> bool
  val saved_state_ignore_hhconfig: options -> bool
  val json_mode: options -> bool
  val load_state_canary: options -> bool
  val log_inference_constraints: options -> bool
  val max_procs: options -> int option
  val no_load: options -> bool
  val prechecked: options -> bool option
  val profile_log: options -> bool
  val replace_state_after_saving: options -> bool
  val root: options -> Path.t
  val save_filename: options -> string option
  val save_with_spec: options -> save_state_spec_info option
  val save_naming_filename: options -> string option
  val should_detach: options -> bool
  val waiting_client: options -> Unix.file_descr option
  val watchman_debug_logging: options -> bool
  val with_saved_state: options -> saved_state_target option
  val allow_non_opt_build: options -> bool

  (****************************************************************************)
  (* Setters *)
  (****************************************************************************)

  val set_gen_saved_ignore_type_errors: options -> bool -> options
  val set_saved_state_target: options -> ServerMonitorUtils.target_saved_state option -> options
  val set_no_load: options -> bool -> options
  val set_config: options -> (string * string) list -> options

  (****************************************************************************)
  (* Misc *)
  (****************************************************************************)
  val print_json_version: unit -> unit
  val to_string: options -> string
  val verify_save_with_spec: string option -> save_state_spec_info option
end

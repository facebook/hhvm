module Types = struct

  type mini_state_target = {
    saved_state_fn   : string;
    corresponding_base_revision : string;
    deptable_fn      : string;
    changes          : Relative_path.t list;
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

  val parse_options: unit -> options
  val default_options: root:string -> options
  val print_json_version: unit -> unit

  (****************************************************************************)
  (* Accessors *)
  (****************************************************************************)

  val ai_mode             : options -> Ai_options.t option
  val check_mode          : options -> bool
  val json_mode           : options -> bool
  val root                : options -> Path.t
  val should_detach       : options -> bool
  val convert             : options -> Path.t option
  val max_procs           : options -> int
  val no_load             : options -> bool
  val profile_log         : options -> bool
  val with_mini_state     : options -> mini_state_target option
  val save_filename       : options -> string option
  val use_gen_deps        : options -> bool
  val waiting_client      : options -> Unix.file_descr option
  val debug_client        : options -> Handle.handle option

  (****************************************************************************)
  (* Setters *)
  (****************************************************************************)

  val set_no_load         : options -> bool -> options

end

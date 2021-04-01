(*
 * Put common type definitions outside of the module type signature so it can
 * be included in both the .mli and .ml.
 *)
module Types = struct
  type options = {
    root: Path.t;
    allow_subscriptions: bool;
        (** Disable the informant - use the dummy instead. *)
    use_dummy: bool;
        (** Don't trigger a server restart if the distance between two
     * revisions we are moving between is less than this. *)
    min_distance_restart: int;
        (** Keep at most this many saved states, and delete older ones. *)
    saved_state_cache_limit: int;
        (** Informant should check the XDB table for a saved state when
     * making a decision. *)
    use_xdb: bool;
    watchman_debug_logging: bool;
        (** Informant should ignore the hh_version column when looking up a saved
     * state from XDB. *)
    ignore_hh_version: bool;
        (** Informant should ignore the hhconfig_hash column when looking up a saved
     * state from XDB. *)
    ignore_hhconfig: bool;
        (** Was the server initialized with a precomputed saved-state? *)
    is_saved_state_precomputed: bool;
  }

  type init_env = options
end

module type S = sig
  type t

  include module type of Types

  include Informant_sig.S with type t := t and type init_env := init_env
end

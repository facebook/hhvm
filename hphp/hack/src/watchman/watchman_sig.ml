(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Ppx_yojson_conv_lib.Yojson_conv.Primitives

module Types = struct
  exception Timeout

  exception Watchman_error of string

  exception Subscription_canceled_by_watchman

  exception Watchman_restarted

  type subscribe_mode =
    | All_changes
    | Defer_changes
    | Scm_aware

  type timeout =
    | No_timeout
    | Default_timeout
    | Explicit_timeout of float

  type init_settings = {
    (* None for query mode, otherwise specify subscriptions mode. *)
    subscribe_mode: subscribe_mode option;
    (* Seconds used for init timeout - will be reused for reinitialization. None -> no timeout *)
    init_timeout: timeout;
    (* See watchman expression terms. *)
    expression_terms: Hh_json.json list;
    debug_logging: bool;
    roots: Path.t list;
    sockname: string option;
    subscription_prefix: string;
  }

  (** The message's clock. *)
  type clock = string [@@deriving eq, show]

  type pushed_changes =
    (*
     * State name and metadata.
     *
     * For example:
     *   State name: "hg.update"
     * Metadata:
     *   {
     *    "partial":false,
     *    "rev":"780dab9ff0a01691c9b18a5ee1194810e555c78b",
     *    "distance":2,
     *    "status":"ok"
     *   }
     *
     * Note: The distance is HG Revision distance, not SVN revision distance.
     *)
    | State_enter of string * Hh_json.json option
    | State_leave of string * Hh_json.json option
    | Changed_merge_base of Hg.Rev.t * SSet.t * clock
    | Files_changed of SSet.t
  [@@deriving show]

  type changes =
    | Watchman_unavailable
    | Watchman_pushed of pushed_changes
    | Watchman_synchronous of pushed_changes list

  (** The response from watchman for the `watch` command should contain these fields *)
  type watch_project_response = {
    watch: string;  (** Corresponds to the VCS repo root. *)
    relative_path: string option; [@yojson.option]
        (** The path being watched relative to the `watch` path *)
  }
  [@@deriving of_yojson] [@@yojson.allow_extra_fields]
end

(** The abstract types, and the types that are defined in terms of
 * abstract types must be split out. The reason is left as an exercise
 * to the reader (i.e. try it yourself out a few ways and you'll discover the
 * limitations - whether your strategy is splitting up the definitions
 * into 3 modules "base types, abstract types, dependent types", or
 * if you change this to a functor). *)
module Abstract_types = struct
  type env

  type dead_env

  (* This has to be repeated because they depend on the abstract types. *)
  type watchman_instance =
    | Watchman_dead of dead_env
    | Watchman_alive of env
end

module type S = sig
  include module type of Types

  include module type of Abstract_types

  type conn

  val init : ?since_clockspec:string -> init_settings -> unit -> env option

  val get_all_files : env -> string list

  val get_changes_since_mergebase : ?timeout:timeout -> env -> string list

  val get_mergebase : ?timeout:timeout -> env -> Hg.Rev.t

  val get_changes :
    ?deadline:float -> watchman_instance -> watchman_instance * changes

  val get_changes_synchronously :
    timeout:int -> watchman_instance -> watchman_instance * pushed_changes list

  val get_clock : watchman_instance -> clock

  val conn_of_instance : watchman_instance -> conn option

  val close : env -> unit

  val with_instance :
    watchman_instance ->
    try_to_restart:bool ->
    on_alive:(env -> 'a) ->
    on_dead:(dead_env -> 'a) ->
    'a

  val get_reader : watchman_instance -> Buffered_line_reader.t option

  module RepoStates : sig
    val get_as_telemetry : unit -> Telemetry.t
  end

  (* Expose some things for testing. *)
  module Testing : sig
    val get_test_env : unit -> env

    val test_settings : init_settings

    val transform_asynchronous_get_changes_response :
      env -> Hh_json.json option -> env * pushed_changes
  end

  module Mocking : sig
    val print_env : env -> string

    val init_returns : string option -> unit

    val get_changes_returns : changes -> unit
  end
end

module type Exec = sig
  type 'a future

  type error

  module Monad_infix : sig
    val ( >>| ) : 'a future -> ('a -> 'b) -> 'b future

    val ( >|= ) :
      ('a, 'e) result future ->
      ('a -> ('b, 'e) result) ->
      ('b, 'e) result future
  end

  val exec : Exec_command.t -> string list -> (string, error) result future
end

module type Process_S = sig
  type 'a future

  type exec_error

  type error =
    | Process_failure of exec_error
    | Unexpected_json of { json_string: string }

  (** [watch_project ~root ~socket] queries watchman to watch a [root]. *)
  val watch_project :
    root:Path.t ->
    sockname:Path.t option ->
    (Types.watch_project_response, error) result future
end

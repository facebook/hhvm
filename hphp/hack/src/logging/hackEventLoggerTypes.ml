(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module PerFileProfilingConfig = struct
  type profile_decling =
    | DeclingOff
    | DeclingTopCounts
    | DeclingAllTelemetry of { callstacks: bool }

  module ProfileDecling = struct
    type t = profile_decling

    let of_config_value (s : string) : t option =
      match s with
      | "off" -> Some DeclingOff
      | "top_counts" -> Some DeclingTopCounts
      | "all_telemetry" -> Some (DeclingAllTelemetry { callstacks = false })
      | "all_telemetry_callstacks" ->
        Some (DeclingAllTelemetry { callstacks = true })
      | _ -> None

    let to_config_value (t : t) : string =
      match t with
      | DeclingOff -> "off"
      | DeclingTopCounts -> "top_counts"
      | DeclingAllTelemetry { callstacks } ->
        Printf.sprintf
          "all_telemetry%s"
          (if callstacks then
            "_callstacks"
          else
            "")

    let of_int (i : int) : t =
      match i with
      | 0 -> DeclingOff
      | 1 -> DeclingTopCounts
      | 2 -> DeclingAllTelemetry { callstacks = false }
      | _ -> DeclingAllTelemetry { callstacks = true }

    let config_value_of_int (i : int) : string = i |> of_int |> to_config_value
  end

  type t = {
    profile_log: bool;
    profile_type_check_duration_threshold: float;
    profile_type_check_memory_threshold_mb: int;
    profile_type_check_twice: bool;
    profile_decling: profile_decling;
    profile_owner: string option;
    profile_desc: string option;
    profile_slow_threshold: float;
  }

  let default =
    {
      profile_log = false;
      profile_type_check_duration_threshold = 0.05 (* seconds *);
      profile_type_check_memory_threshold_mb = 100;
      profile_type_check_twice = false;
      profile_decling = DeclingOff;
      profile_owner = None;
      profile_desc = None;
      profile_slow_threshold = Float.max_value;
    }
end

type rollout_flags = {
  log_saved_state_age_and_distance: bool;
      (** POC: @nzthomas. Collects the age of a saved state (in seconds)
          and distance (in globalrevs) for telemetry *)
  fetch_remote_old_decls: bool;
      (** POC: @bobren - Fetching old decls remotely to reduce fanout *)
  specify_manifold_api_key: bool;
      (** POC: @nzthomas, whether the API key in hh.conf should be used for saved state downloads *)
  populate_member_heaps: bool;
      (** POC: @hverr, disable writing signatures to method/property heaps. *)
  shm_use_sharded_hashtbl: bool;  (** POC: @hverr, new shared memory backend. *)
  shm_cache_size: int;
      (** POC: @hverr, evictability cache size, -1 if evictability disabled *)
  remote_old_decls_no_limit: bool;
      (** POC: @bobren, Remove remote old decl fetching limit *)
  use_manifold_cython_client: bool;
      (** POC: @nzthomas, Required for Hedwig support for saved state downloads *)
  disable_naming_table_fallback_loading: bool;
      (** POC: @nzthomas, Stop falling back to OCaml marshalled naming table when sqlite table is missing *)
  rust_provider_backend: bool;
      (** POC: @jakebailey, started Dec 2022.
          Use Provider_backend.Rust_provider_backend as the global provider
          backend, servicing File_provider, Naming_provider, and Decl_provider
          queries using the hackrs implementation. *)
  use_distc: bool;
      (** POC: @bobren, use hh_distc instead of hulk for remote typechecking *)
  consume_streaming_errors: bool;
      (** POC: @ljw, hh_client should read errors from errors.bin *)
  hh_distc_fanout_threshold: int;
      (** POC: @bobren - fanout threshold where we trigger hh_distc *)
  rust_elab: bool;
      (** POC: @mjt, use unified-elaboration rather than hh naming/nast checks *)
  ide_load_naming_table_on_disk: bool;
      (** POC: @nzthomas - allow ClientIdeDaemon to grab any naming table from disk before trying Watchman / Manifold *)
  ide_naming_table_update_threshold: int;
      (** POC: @nzthomas, if clientIDEDaemon is loading a naming table from disk instead of Manifold, set a globalrev distance threshold *)
  use_compressed_dep_graph: bool;
      (** POC: @bobren, use new fancy compressed dep graph that is 25% the size of the old one *)
  load_state_natively_v4: bool;
      (** POC: @bobren, whether to allow hh_server to fetch saved state (thus overriding load_state_natively setting on Sandcastle) *)
  saved_state_rollouts: Saved_state_rollouts.t;
  zstd_decompress_by_file: bool;
      (** POC: @nzthomas, if true then saved state loading will only pass specific files to zstd shellout *)
  lsp_sticky_quarantine: bool;
      (** POC: @ljw - if true, only exit quarantine when entering a new one *)
  lsp_invalidation: bool;
      (** POC: @ljw - relates to how we invalidate folded decls for quarantine *)
  invalidate_all_folded_decls_upon_file_change: bool;  (** POC @catg *)
  autocomplete_sort_text: bool;
      (** POC: @mckenzie - if true, autocomplete results will be sorted using sort text attribute *)
  warnings_default_all: bool;
      (** POC: @catg - whether the client shows warnings by default. *)
  improved_hover: bool;  (** POC: @catg *)
}

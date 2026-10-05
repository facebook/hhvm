(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Which files a run starts growing from. A whole-repository scan finds the
    files nothing references; a seed list or an entry-point family names them
    instead, and those may well have references in — the cluster grown around
    such a seed is what the question is about. *)
module Seeds = struct
  (** A file is a seed when nothing outside it depends on any symbol it defines,
      compared in hash space so no naming-table lookup is needed. Only top-level
      hashes are known, so a dependent recorded at member granularity inside the
      file also counts as outside: the test can withhold seed status from a file
      that deserves it, never grant it to one that does not. *)
  let has_external_dependents deps_mode file_info =
    let own =
      Typing_deps.deps_of_file_info file_info |> Typing_deps.DepSet.of_list
    in
    let dependents = Typing_deps.add_typing_deps deps_mode own in
    not (Typing_deps.DepSet.is_empty (Typing_deps.DepSet.diff dependents own))

  (** Paths the typechecker already excludes, per [package_exclude_patterns].

      A test is the perfect seed and a useless one — nothing depends on it, so it
      always passes the seed test — and on a whole-repository scan they are a large
      share of what is found. This filters where seeds are scanned for, not what
      may end up inside a cluster: a test still has to join the cluster holding the
      code it exercises, since strict isolation switches off the exemption these
      patterns otherwise grant.

      Returns the test rather than performing it, so the patterns compile once
      rather than once per file in the repository. *)
  let excluded_path_filter ctx =
    let patterns =
      Provider_context.get_tcopt ctx
      |> Typechecker_options.package_exclude_patterns
      |> List.map ~f:Str.regexp
    in
    fun path ->
      let filename = Relative_path.to_absolute path in
      List.exists patterns ~f:(fun pattern ->
          Str.string_match pattern filename 0)

  let scan ctx deps_mode naming_table =
    let is_excluded_path = excluded_path_filter ctx in
    (* One whole-repo naming table scan, unavoidable for a whole-repo query. The
       [file_info] it yields is used in place; re-deriving it per file would cost
       a SQLite SELECT each. *)
    Naming_table.fold
      ~warn_on_naming_costly_iter:false
      naming_table
      ~init:[]
      ~f:(fun path file_info seeds ->
        if
          Relative_path.is_root (Relative_path.prefix path)
          && (not (is_excluded_path path))
          && not (has_external_dependents deps_mode file_info)
        then
          path :: seeds
        else
          seeds)
    |> List.sort ~compare:Relative_path.compare

  (** Every subclass of [base], less the base itself: it defines the family rather
      than belonging to it, and is the one member nothing can isolate.

      [add_extend_deps] walks inheritance recursively, so one call gives the whole
      family rather than direct children. The name is normalised to a leading
      backslash, which is how the graph spells a top-level class and not how a
      caller types one. *)
  let from_framework ctx deps_mode base =
    let base =
      if String.is_prefix base ~prefix:"\\" then
        base
      else
        "\\" ^ base
    in
    let root = Typing_deps.(DepSet.singleton (Dep.make (Dep.Type base))) in
    let family = Typing_deps.add_extend_deps deps_mode root in
    let is_excluded_path = excluded_path_filter ctx in
    let base_files = Naming_provider.get_files ctx root in
    let paths =
      Naming_provider.get_files ctx family
      |> Relative_path.Set.elements
      |> List.filter ~f:(fun path ->
             Relative_path.is_root (Relative_path.prefix path)
             && (not (is_excluded_path path))
             && not (Relative_path.Set.mem base_files path))
    in
    Hh_logger.log
      "[isolation] seed framework: %s, %d subclasses"
      base
      (List.length paths);
    if List.is_empty paths then
      Hh_logger.log
        "[isolation] seed framework: %s resolved to nothing — is the name right, and fully qualified?"
        base;
    paths

  (** Order preserved, so a seed window over a list without repeats selects the
      same slice as before. *)
  let dedupe paths =
    List.fold
      paths
      ~init:([], Relative_path.Set.empty)
      ~f:(fun (acc, seen) path ->
        if Relative_path.Set.mem seen path then
          (acc, seen)
        else
          (path :: acc, Relative_path.Set.add seen path))
    |> fst
    |> List.rev

  (** The repo-relative paths in [file], one per line.

      A path the naming table does not know is dropped rather than grown from: it
      would look like a file that references nothing, and report as an isolatable
      cluster of one. Silently wrong beats loudly wrong here, so it is counted out
      loud instead. A repeat would be grown twice and the second cluster discarded
      for overlapping the first, which is indistinguishable in the counters from
      two distinct seeds colliding. *)
  let from_list naming_table file =
    let lines =
      In_channel.read_lines file
      |> List.filter_map ~f:(fun line ->
             match String.strip line with
             | "" -> None
             | suffix -> Some (Relative_path.from_root ~suffix))
    in
    let deduped = dedupe lines in
    let repeated = List.length lines - List.length deduped in
    if repeated > 0 then
      Hh_logger.log "[isolation] seed list: %d repeated paths ignored" repeated;
    let (known, unknown) =
      List.partition_tf deduped ~f:(fun path ->
          Option.is_some (Naming_table.get_file_info naming_table path))
    in
    Hh_logger.log
      "[isolation] seed list: %d paths from %s, %d unknown to the naming table"
      (List.length known)
      file
      (List.length unknown);
    known

  let select ctx deps_mode naming_table ~seed_framework ~seed_list =
    match (seed_framework, seed_list) with
    | (Some base, _) -> from_framework ctx deps_mode base
    | (None, Some file) -> from_list naming_table file
    | (None, None) ->
      let seeds = scan ctx deps_mode naming_table in
      Hh_logger.log "[isolation] seed scan: found %d seeds" (List.length seeds);
      seeds

  let window all_seeds ~seed_offset ~max_seeds =
    let seeds = List.drop all_seeds seed_offset in
    match max_seeds with
    | None -> seeds
    | Some n -> List.take seeds n

  let log_window ~total_seeds ~seed_offset ~max_seeds ~available =
    if seed_offset > 0 || Option.is_some max_seeds then
      Hh_logger.log
        "[isolation] seed window: %d seeds from offset %d (of %d)"
        total_seeds
        seed_offset
        available
end

(** What a query threads through: the providers it reads and the tally a mode
    keeps as it goes. A mode is what fills the answer in; until one exists a
    run reports its seeds and nothing more. *)
type run = {
  ctx: Provider_context.t;
  deps_mode: Typing_deps_mode.t;
  naming_table: Naming_table.t;
  truncated_clusters: int ref;
      (** Sets a mode stopped short of closing, so they are prefixes of
          packages rather than packages. Counted so a run says how much of its
          output is in that state. *)
}

let make_run _options _genv env =
  let ctx = Provider_utils.ctx_from_server_env env in
  {
    ctx;
    deps_mode = Provider_context.get_deps_mode ctx;
    naming_table = env.Server_env.naming_table;
    truncated_clusters = ref 0;
  }

(** Each mode reports the sizes of the sets it found, and differs in nothing
    else the caller sees, so the result is assembled here rather than at the
    end of each branch. *)
let result r ~grown ~total_seeds ~sizes ~clusters =
  Server_isolation_types.
    {
      clusters;
      grown;
      total_seeds;
      total_isolatable_files = List.fold sizes ~init:0 ~f:( + );
      total_clusters = List.length sizes;
      total_truncated = !(r.truncated_clusters);
      largest_cluster = List.fold sizes ~init:0 ~f:Int.max;
    }

(** Choose the seeds, then say what was found around them. *)
let query r options =
  let Server_isolation_types.
        { seed_framework; seed_list; max_seeds; seed_offset; _ } =
    options
  in
  let t_start = Unix.gettimeofday () in
  let all_seeds =
    Seeds.select r.ctx r.deps_mode r.naming_table ~seed_framework ~seed_list
  in
  let (_ : float) = Hh_logger.log_duration "[isolation] seed scan" t_start in
  let seeds = Seeds.window all_seeds ~seed_offset ~max_seeds in
  let total_seeds = List.length seeds in
  Seeds.log_window
    ~total_seeds
    ~seed_offset
    ~max_seeds
    ~available:(List.length all_seeds);
  result r ~grown:false ~total_seeds ~sizes:[] ~clusters:[]

let go
    (options : Server_isolation_types.options)
    (genv : Server_env.genv)
    (env : Server_env.env) : Server_isolation_types.result =
  (* Non-evictable shared heaps that outlive the query: cleared on the way in
     so a run cannot reuse results from before an edit, and on the way out so
     it leaves nothing resident. *)
  let reset_caches () =
    Server_isolation_outbound.reset ();
    Server_isolation_inbound.reset ()
  in
  Utils.try_finally ~finally:reset_caches ~f:(fun () ->
      let r = make_run options genv env in
      reset_caches ();
      query r options)

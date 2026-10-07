(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Growth = Server_isolation_growth
module Memory = Server_isolation_memory

(* Read top to bottom: where a run starts ([Seeds]) and where its clusters go
   ([Sink]); the [run] record both modes thread through; then the two modes,
   each seed's closure first and clusters grown around them second; and last
   [make_run], [result], [query] and [go], which choose a mode and assemble the
   answer. *)

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

(** Where a cluster goes as soon as it is known. In memory for a small run; one
    JSON object per line for a large one, where carrying every cluster back
    through the RPC would put them all in memory before the server could
    reply. *)
module Sink = struct
  type t = {
    emit: Server_isolation_types.cluster -> unit;
    collected: unit -> Server_isolation_types.cluster list;
    close: unit -> unit;
  }

  (* The leading components two paths agree on. *)
  let common_prefix a b =
    let (paired, _) = List.zip_with_remainder a b in
    List.take_while paired ~f:(fun (x, y) -> String.equal x y)
    |> List.map ~f:fst

  (** Longest directory prefix shared by every file, "" when there is none. *)
  let common_directory (files : Relative_path.t list) : string =
    let dirs =
      List.map files ~f:(fun path ->
          (* [Filename.dirname] answers "." for a file at the root, which would
             otherwise render as a "./" prefix. *)
          match Relative_path.suffix path |> Filename.dirname with
          | "." -> []
          | dir -> String.split dir ~on:'/')
    in
    match dirs with
    | [] -> ""
    | first :: rest ->
      (match List.fold rest ~init:first ~f:common_prefix with
      | [] -> ""
      | shared -> String.concat ~sep:"/" shared ^ "/")

  let in_memory () =
    let acc = ref [] in
    {
      emit = (fun cluster -> acc := cluster :: !acc);
      collected = (fun () -> List.rev !acc);
      close = (fun () -> ());
    }

  (* [grown] is recorded on every line because both modes write sets of files
     called clusters to the same place, and they mean different things: the
     smallest isolatable set around a file, or that set grown as far as the bounds
     allow. A file that says which can be read without knowing the command that
     produced it. *)
  let to_file ~grown path =
    let out = Stdlib.open_out path in
    {
      emit =
        (fun cluster ->
          let open Server_isolation_types in
          `Assoc
            [
              ("files", `List (List.map cluster.files ~f:(fun f -> `String f)));
              ("size", `Int (List.length cluster.files));
              ("common_directory", `String cluster.common_directory);
              ("grown", `Bool grown);
              ("truncated", `Bool cluster.truncated);
            ]
          |> Yojson.Safe.to_string
          |> fun line ->
          Stdlib.output_string out line;
          Stdlib.output_char out '\n';
          (* Flushed per cluster, not left to [close_out]: a family-sized run
             can be killed partway, and a buffer the process never writes takes
             the last clusters with it. *)
          Stdlib.flush out);
      (* The caller is reading the file, so returning them as well would put back
         the memory this exists to avoid. *)
      collected = (fun () -> []);
      close = (fun () -> Stdlib.close_out out);
    }
end

open Sink

(** What a query threads through: the providers it reads, the bounds it runs
    under, where its clusters go, and the two tallies only the caller of growth
    can keep. *)
type run = {
  ctx: Provider_context.t;
  workers: Multi_worker.worker list option;
  deps_mode: Typing_deps_mode.t;
  naming_table: Naming_table.t;
  sink: Sink.t;
  max_dependents: int;
  max_cluster_size: int option;
  overlapping_clusters: int ref;
      (** Clusters dropped because an earlier cluster in the same batch already
          claimed one of their files. Counted here rather than in growth: growth
          reports each cluster it grew, and whether two of them overlap is only
          knowable once they are collected. *)
  truncated_clusters: int ref;
      (** Clusters the cap refused a candidate they would otherwise have taken.
          They stopped short of closing, so they are prefixes of packages rather
          than packages, and a consumer that treats one as finished will find
          files outside it referencing in. Counted so a run says how much of its
          output is in that state. *)
}

let median sizes =
  match List.sort sizes ~compare:Int.compare with
  | [] -> 0
  | sorted -> List.nth_exn sorted (List.length sorted / 2)

let histogram sizes ~bucket ~label =
  List.fold sizes ~init:Int.Map.empty ~f:(fun acc n ->
      Map.update acc (bucket n) ~f:(function
          | None -> 1
          | Some c -> c + 1))
  |> Map.to_alist
  |> List.map ~f:(fun (b, count) -> label b count)
  |> String.concat ~sep:" "

(* Bucket boundaries for the size histogram: fine at the bottom, where most
   closures fall, and wide at the top, where they span orders of magnitude. A
   list of raw sizes carries the same information and shows none of its shape. *)
let closure_buckets = [1; 2; 5; 10; 50; 100; 500; 2000]

(* Anything past the last bucket shares one. [max_value] keys it so it sorts
   last, and is never printed — the label says what it actually means. *)
let closure_bucket n =
  List.find closure_buckets ~f:(fun bucket -> n <= bucket)
  |> Option.value ~default:Int.max_value

(* [closure_buckets] holds upper bounds only, so a range starts one past the
   previous bound: 1, 2, 3-5, 6-10, and so on. *)
let closure_ranges =
  List.folding_map closure_buckets ~init:1 ~f:(fun lower upper ->
      (upper + 1, (lower, upper)))

let closure_label bucket count =
  if Int.equal bucket Int.max_value then
    Printf.sprintf "%d+:%d" (List.last_exn closure_buckets + 1) count
  else
    match
      List.find closure_ranges ~f:(fun (_, upper) -> Int.equal upper bucket)
    with
    | Some (lower, upper) when Int.equal lower upper ->
      Printf.sprintf "%d:%d" upper count
    | Some (lower, upper) -> Printf.sprintf "%d-%d:%d" lower upper count
    | None -> Printf.sprintf "%d:%d" bucket count

(** One seed's closure, emitted and released; [None] when the walk refused it. *)
let closure_of_seed r path =
  match
    Server_isolation_inbound.get r.ctx r.naming_table ~max:r.max_dependents path
  with
  | None -> None
  | Some closure ->
    let files = Relative_path.Set.elements closure in
    r.sink.emit
      Server_isolation_types.
        {
          files = List.map files ~f:Relative_path.suffix;
          common_directory = common_directory files;
          (* Nothing grew, so no cap refused anything. *)
          truncated = false;
        };
    Server_isolation_inbound.release_closures ();
    Some (List.length files)

(** Reported as the run goes so one that is killed still says where the memory
    was going, which the single line at the end cannot. *)
let log_closure_progress ~done_count ~total_seeds ~fitted_count =
  Hh_logger.log
    "[isolation] closure report: %d/%d seeds | %d isolatable | heap %.1fGiB rss %.1fGiB"
    done_count
    total_seeds
    fitted_count
    (Memory.heap_gib ())
    (Memory.rss_gib ())

let log_closure_report ~total_seeds ~closure_sizes =
  Hh_logger.log
    "[isolation] closure report: %d files | %d isolatable (%.1f%%) | median %d | largest %d | buckets %s"
    total_seeds
    (List.length closure_sizes)
    (100.0
    *. float_of_int (List.length closure_sizes)
    /. float_of_int (max 1 total_seeds))
    (median closure_sizes)
    (List.fold closure_sizes ~init:0 ~f:Int.max)
    (histogram closure_sizes ~bucket:closure_bucket ~label:closure_label)

(** Measure each seed's isolatability: walk its closure, emit it, and return
    the sizes. The closure is what "is this file isolatable, and what would it
    cost" means. A refused seed counts as not isolatable, which is the
    question's own definition — except where the walk's hash guard refuses a
    closure that would have fitted in files, which is why these figures are a
    lower bound.

    Emitted and released one at a time; only the sizes are needed after.
    Collected in batches because the walk's dependency sets live behind a Rust
    custom block, so the OCaml collector barely notices them. [compact] rather
    than [full_major] to hand memory back to the operating system. *)
let measure_seeds r seeds ~total_seeds =
  (* Seeds between forced compactions. Large enough that walking the whole heap
     is not what the run spends its time on. Not measured. *)
  let compact_batch_size = 2000 in
  (* On a clock rather than on the batch: a family smaller than one batch would
     otherwise report nothing until it finished. *)
  let seconds_between_reports = 60.0 in
  let done_count = ref 0 in
  let fitted_count = ref 0 in
  let last_report = ref (Unix.gettimeofday ()) in
  let report () =
    last_report := Unix.gettimeofday ();
    log_closure_progress
      ~done_count:!done_count
      ~total_seeds
      ~fitted_count:!fitted_count
  in
  let report_if_due () =
    if Float.(Unix.gettimeofday () -. !last_report > seconds_between_reports)
    then
      report ()
  in
  let measure_one_seed seed =
    let size = closure_of_seed r seed in
    incr done_count;
    if Option.is_some size then incr fitted_count;
    report_if_due ();
    size
  in
  let measure_one_batch batch =
    let measured_sizes = List.filter_map batch ~f:measure_one_seed in
    Gc.compact ();
    measured_sizes
  in
  let measured_sizes =
    List.chunks_of seeds ~length:compact_batch_size
    |> List.concat_map ~f:measure_one_batch
  in
  report ();
  log_closure_report ~total_seeds ~closure_sizes:measured_sizes;
  measured_sizes

let log_growth_progress ~grown ~total_seeds ~since =
  let Growth.Stats.
        {
          index_seconds;
          absorb_seconds;
          candidates_tested;
          absorbed_by_closure;
          _;
        } =
    Growth.stats ()
  in
  Hh_logger.log
    "[isolation] growth: %d/%d seeds | index %.1fs (%d files) | absorb %.1fs (%d candidates, %d by closure) | closures %d computed %d served %d refused | heap %.1fGiB | rss %.1fGiB | total %.1fs"
    grown
    total_seeds
    index_seconds
    (Server_isolation_outbound.files_indexed ())
    absorb_seconds
    candidates_tested
    absorbed_by_closure
    (Server_isolation_inbound.closures_computed ())
    (Server_isolation_inbound.closures_served ())
    (Server_isolation_inbound.refusals_held ())
    (Memory.heap_gib ())
    (Memory.rss_gib ())
    (Unix.gettimeofday () -. since)

(** Logged as well as returned, since a long run outlives its client. *)
let log_growth_result r ~sizes =
  let Growth.Stats.{ absorbed_by_subset; absorbed_by_closure; _ } =
    Growth.stats ()
  in
  Hh_logger.log
    "[isolation] result: %d clusters (%d truncated) | %d files | median %d | largest %d | subset %d closure %d | dropped %d | sizes %s"
    (List.length sizes)
    !(r.truncated_clusters)
    (List.fold sizes ~init:0 ~f:( + ))
    (median sizes)
    (List.fold sizes ~init:0 ~f:Int.max)
    absorbed_by_subset
    absorbed_by_closure
    !(r.overlapping_clusters)
    (histogram sizes ~bucket:Fn.id ~label:(Printf.sprintf "%d:%d"))

(** One grown cluster against what the clusters before it took. Clusters in one
    batch share a [claimed] snapshot, so two can take the same file. Trimming
    the overlap out of one would leave it with an inbound edge from the file it
    lost, so the later cluster is dropped whole. *)
let collect_cluster
    r
    (clusters, visited, claimed)
    Growth.{ seed = _; files = cluster; truncated } =
  if not (Relative_path.Set.is_empty (Relative_path.Set.inter cluster claimed))
  then begin
    (* [visited] is left alone. A dropped cluster was never reported, so its
       files are in no package, and marking them would retire seeds that no
       output accounts for — a later batch tests against a larger [claimed] and
       may grow one of them into a cluster that does not overlap. *)
    incr r.overlapping_clusters;
    (clusters, visited, claimed)
  end else begin
    let visited = Relative_path.Set.union visited cluster in
    let files = Relative_path.Set.elements cluster in
    let claimed = Relative_path.Set.union claimed cluster in
    if truncated then incr r.truncated_clusters;
    r.sink.emit
      Server_isolation_types.
        {
          files = List.map files ~f:Relative_path.suffix;
          common_directory = Sink.common_directory files;
          truncated;
        };
    (List.length files :: clusters, visited, claimed)
  end

(** One batch grown and collected, then everything the next batch will not ask
    for again released. *)
let grow_one_batch
    r
    ~max_dependents
    ~total_seeds
    ~since
    ~grown
    (clusters, visited, claimed)
    batch =
  (* A seed already absorbed by an earlier cluster is not its own cluster, but it
     still counts towards progress: the denominator is every seed, not just the
     ones that needed growing. *)
  grown := !grown + List.length batch;
  let batch =
    List.filter batch ~f:(fun seed -> not (Relative_path.Set.mem visited seed))
  in
  let grown_batch =
    Growth.grow_batch
      r.ctx
      r.workers
      r.deps_mode
      r.naming_table
      ~claimed
      ~max_cluster_size:r.max_cluster_size
      ~max_dependents
      batch
  in
  log_growth_progress ~grown:!grown ~total_seeds ~since;
  (* This batch's clusters are at a fixed point, so nothing will ask for their
     outbound edges again, and an accepted closure is cheap to recompute if a
     later batch does. Refusals survive: they cost a full walk and the files that
     earn them recur everywhere. *)
  Server_isolation_outbound.release_cache ();
  Server_isolation_inbound.release_closures ();
  Growth.release_caches ();
  List.fold
    grown_batch
    ~init:(clusters, visited, claimed)
    ~f:(collect_cluster r)

(** Seeds are advanced in batches so each round's frontiers pool into one
    parallel index call. Larger batches fill the worker pool better; they also
    hold more clusters in memory at once. A few thousand keeps both in hand. *)
let grow_seeds r seeds ~total_seeds ~since =
  (* A closure larger than the whole cluster can never be absorbed, and the walk
     is where growth spends its time. Clamped once per run, not per candidate:
     the memo is keyed by path, so a varying bound would poison it. Growth only —
     under [no_growth] the bound is the measurement. *)
  let max_dependents =
    match r.max_cluster_size with
    | Some cap -> min r.max_dependents cap
    | None -> r.max_dependents
  in
  Hh_logger.log
    "[isolation] cluster growth: starting over %d seeds (closure bound %d)"
    total_seeds
    max_dependents;
  (* Seeds grown in lockstep. The batch is what pools candidates into one
     parallel indexing call — a single cluster's frontier would leave the worker
     pool idle — and every cluster in it is live at once, so this is peak memory
     against worker occupancy. Not measured: the family-sized runs used windows
     of 400 seeds, which makes a window one batch. *)
  let batch_size = 2000 in
  let grown = ref 0 in
  (* [visited] stops a seed being grown twice; [claimed] is every file in a
     cluster already reported. *)
  let (clusters, _visited, _claimed) =
    List.fold
      (List.chunks_of seeds ~length:batch_size)
      ~init:([], Relative_path.Set.empty, Relative_path.Set.empty)
      ~f:(grow_one_batch r ~max_dependents ~total_seeds ~since ~grown)
  in
  let _t = Hh_logger.log_duration "[isolation] cluster growth" since in
  let sizes = List.rev clusters in
  log_growth_result r ~sizes;
  sizes

let make_run options genv env sink =
  let Server_isolation_types.{ max_dependents; max_cluster_size; _ } =
    options
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  {
    ctx;
    workers = genv.Server_env.workers;
    deps_mode = Provider_context.get_deps_mode ctx;
    naming_table = env.Server_env.naming_table;
    sink;
    max_dependents;
    max_cluster_size;
    overlapping_clusters = ref 0;
    truncated_clusters = ref 0;
  }

(** Each mode reports the sizes of the sets it found, and differs in nothing
    else the caller sees, so the result is assembled here rather than at the end
    of each branch. *)
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

(** The query itself: which files to start from, then how far to take them. Runs
    with the caches already clear and the sink already open. *)
let query r options =
  let Server_isolation_types.
        { no_growth; seed_framework; seed_list; max_seeds; seed_offset; _ } =
    options
  in
  (* Accumulates across a run; a second query in the same server would otherwise
     report the first one's totals. *)
  Growth.reset_stats ();
  let t_start = Unix.gettimeofday () in
  let all_seeds =
    Seeds.select r.ctx r.deps_mode r.naming_table ~seed_framework ~seed_list
  in
  let t = Hh_logger.log_duration "[isolation] seed scan" t_start in
  let seeds = Seeds.window all_seeds ~seed_offset ~max_seeds in
  let total_seeds = List.length seeds in
  Seeds.log_window
    ~total_seeds
    ~seed_offset
    ~max_seeds
    ~available:(List.length all_seeds);
  let sizes =
    if no_growth then
      measure_seeds r seeds ~total_seeds
    else
      grow_seeds r seeds ~total_seeds ~since:t
  in
  result
    r
    ~grown:(not no_growth)
    ~total_seeds
    ~sizes
    ~clusters:(r.sink.collected ())

let go
    (options : Server_isolation_types.options)
    (genv : Server_env.genv)
    (env : Server_env.env) : Server_isolation_types.result =
  let Server_isolation_types.{ no_growth; output_file; _ } = options in
  let sink =
    match output_file with
    | Some path -> Sink.to_file ~grown:(not no_growth) path
    | None -> Sink.in_memory ()
  in
  (* Non-evictable shared heaps that outlive the query: cleared on the way in so
     a run cannot reuse results from before an edit, and on the way out so it
     leaves nothing resident. *)
  let reset_caches () =
    Server_isolation_outbound.reset ();
    Server_isolation_inbound.reset ();
    Growth.release_caches ()
  in
  (* Closed on the way out only — [reset_caches] also runs at the start, and
     closing there would leave every cluster written to a closed channel. *)
  Utils.try_finally
    ~finally:(fun () ->
      (* Nested, so that a sink failing to flush cannot skip the cache reset:
         those heaps are non-evictable and would outlive the query. *)
      Utils.try_finally ~f:sink.close ~finally:reset_caches)
    ~f:(fun () ->
      let r = make_run options genv env sink in
      reset_caches ();
      query r options)

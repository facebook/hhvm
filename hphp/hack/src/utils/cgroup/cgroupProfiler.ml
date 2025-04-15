open Hh_prelude

(** `cgroup_watcher_start filename subtract_kb_for_array` will reset cgroup_watcher counters, and have it
start monitoring the two filenames (e.g. "/sys/fs/.../memory.current" and ".../memory.swap.current")
and adds them together. See [cgroup_watcher_get] for the meaning of [subtract_kb_for_array]. *)
external cgroup_watcher_start :
  string -> string -> subtract_kb_for_array:int -> unit = "cgroup_watcher_start"

(** This returns (hwm_kb, num_readings, seconds_at_gb[||]) that have been tallied since
[cgroup_watcher_start]. Hwm_kb is the high water mark of the cgroup. Num_readings is the
number of readings that succeeded. The meaning of seconds_at_gb.(i) is that we spent
this many seconds with the value between (subtract_kb_for_array+i) and
(subtract_kb_for_array+i+1) gb. *)
external cgroup_watcher_get : unit -> int * int * float array
  = "cgroup_watcher_get"

type initial_reading =
  HackEventLogger.ProfileTypeCheck.stats * (CGroup.stats, string) result

(** This is the [initial_reading] that we capture upon module load, i.e. process startup *)
let initial_reading_capture : initial_reading =
  let stats =
    HackEventLogger.ProfileTypeCheck.get_stats
      ~include_current_process:false
      ~include_slightly_costly_stats:true
      ~shmem_heap_size:0
      (Telemetry.create ())
  in
  (stats, CGroup.get_stats ())

(** This is the [initial_reading] that we'll actually use, i.e. we'll record data relative to this.
None means we won't use anything. *)
let initial_reading : initial_reading option ref = ref None

let get_initial_reading () : initial_reading =
  match !initial_reading with
  | None -> initial_reading_capture
  | Some initial_reading -> initial_reading

let use_initial_reading (new_initial_reading : initial_reading) : unit =
  initial_reading := Some new_initial_reading

let get_initial_stats () = get_initial_reading () |> fst

(** A small helper: returns either (initial_reading, f initial_reading) if (1) someone
has previously called [use_initial_reading] and provided an Ok one, (2) the [current_cgroup]
parameter is Ok, (3) the [current_cgroup] has the same cgroup name as the initial one.
Otherwise, it returns ({0...}, default). *)
let initial_value_map current_cgroup ~f ~default =
  let open CGroup in
  match (!initial_reading, current_cgroup) with
  | (Some (_stats, Ok initial_cgroup), Ok current_cgroup)
    when String.equal initial_cgroup.cgroup_name current_cgroup.cgroup_name ->
    (initial_cgroup, f initial_cgroup)
  | _ ->
    ( {
        CGroup.total = 0;
        memory_current = 0;
        memory_swap_current = 0;
        anon = 0;
        shmem = 0;
        file = 0;
        cgroup_name = "";
      },
      default )

type step_summary = {
  time: float;
  log_label_and_suffix: string;
  step_total: int;
}

type step_group = {
  name: string;  (** e.g. "lazy_init" *)
  step_count: int ref;
      (** used to prefix the step-names for telemetry+logging *)
  log: bool;  (** should it be logged to Hh_logger? *)
  last_printed_total: int ref;
      (** we only print to Hh_logger if it's markedly changed since last we printed. *)
  prev_step_to_show: step_summary option ref;
      (** in case we print, we might want to include something about the previous one, if it hasn't already been printed *)
}

(** We'll only bother writing to the log if the total has changed by at least this many bytes *)
let threshold_for_logging = 100 * 1024 * 1024

(** to avoid flooding logs with errors *)
let has_logged_error = ref SSet.empty

(** prints bytes in gb *)
let pretty_num i = Printf.sprintf "%0.2fGiB" (float i /. 1073741824.0)

(** Records to Hh_logger if it differs from the previous total, and update total *)
let log_cgroup_total cgroup ~step_group ~step ~suffix ~total_hwm =
  let (initial, initial_msg) =
    initial_value_map cgroup ~default:"" ~f:(fun i ->
        Printf.sprintf " (relative to initial %s)" (pretty_num i.CGroup.total))
  in
  match cgroup with
  | Error _ -> ()
  | Ok cgroup ->
    let log_label_and_suffix =
      Printf.sprintf "%s/%s%s" step_group.name step suffix
    in
    let prev = !(step_group.prev_step_to_show) in
    let current =
      {
        time = Unix.gettimeofday ();
        log_label_and_suffix;
        step_total = cgroup.CGroup.total;
      }
    in
    if
      abs (cgroup.CGroup.total - !(step_group.last_printed_total))
      < threshold_for_logging
    then
      (* We didn't print this current step. But if the *next* step proves to have a big
         increase in memory, then when we print the next step, we'll also print this one. *)
      step_group.prev_step_to_show := Some current
    else begin
      (* This step is being shown right now. So if the next step proves to have a big
         increase in memory, we won't need to show this one again! *)
      step_group.prev_step_to_show := None;
      step_group.last_printed_total := cgroup.CGroup.total;
      let hwm =
        if cgroup.CGroup.total >= total_hwm then
          ""
        else
          Printf.sprintf
            " (hwm %s)"
            (pretty_num (total_hwm - initial.CGroup.total))
      in
      let prev =
        match prev with
        | None -> ""
        | Some prev ->
          Printf.sprintf
            "\n   >%s %s  [@%s]  previous measurement"
            (Utils.timestring prev.time)
            (pretty_num (prev.step_total - initial.CGroup.total))
            prev.log_label_and_suffix
      in
      Hh_logger.log
        "Cgroup: %s%s  [@%s]%s%s"
        (pretty_num (cgroup.CGroup.total - initial.CGroup.total))
        hwm
        current.log_label_and_suffix
        initial_msg
        prev
    end

(** Given a float array [secs_at_gb] where secs_at_gb[0] says how many secs were spent at cgroup memory 0gb,
secs_at_gb[1] says how many secs were spent at cgroup memory 1gb, and so on, produces an SMap
where keys are some arbitrary thresholds "secs_above_20gb" and values are (int) number of seconds
spent at that memory or higher. We pick just a few arbitrary thresholds that we think are useful
for telemetry, and their sole purpose is telemetry. *)
let secs_above_gb_summary (secs_at_gb : float array) : int SMap.t =
  if Array.is_empty secs_at_gb then
    SMap.empty
  else
    List.init 15 ~f:(fun i -> i * 5) (* 0gb, 5gb, ..., 70gb) *)
    |> List.filter_map ~f:(fun threshold ->
           let title = Printf.sprintf "secs_above_%02dgb" threshold in
           let secs =
             Array.foldi secs_at_gb ~init:0. ~f:(fun i acc s ->
                 acc
                 +.
                 if i < threshold then
                   0.
                 else
                   s)
           in
           let secs = Float.round secs |> int_of_float in
           Option.some_if (secs > 0) (title, secs))
    |> SMap.of_list

(** Records to HackEventLogger *)
let log_telemetry
    ~step_group
    ~step
    ~start_time
    ~total_hwm
    ~start_cgroup
    ~cgroup
    ~telemetry_ref
    ~secs_at_total_gb =
  let open CGroup in
  let (initial, initial_opt) =
    initial_value_map cgroup ~default:false ~f:(fun _ -> true)
  in
  match (start_cgroup, cgroup) with
  | (Some (Error e), _)
  | (_, Error e) ->
    telemetry_ref := Telemetry.create () |> Telemetry.error ~e |> Option.some;
    if SSet.mem e !has_logged_error then
      ()
    else begin
      has_logged_error := SSet.add e !has_logged_error;
      HackEventLogger.CGroup.error e
    end
  | (Some (Ok start_cgroup), Ok cgroup) ->
    let total_hwm = max (max start_cgroup.total cgroup.total) total_hwm in
    let secs_above_total_gb_summary = secs_above_gb_summary secs_at_total_gb in
    let sysinfo = Sys_utils.sysinfo () in
    telemetry_ref :=
      Telemetry.create ()
      |> SMap.fold
           (fun key value -> Telemetry.int_ ~key ~value)
           secs_above_total_gb_summary
      |> Telemetry.int_
           ~key:"start_bytes"
           ~value:(start_cgroup.total - initial.total)
      |> Telemetry.int_ ~key:"end_bytes" ~value:(cgroup.total - initial.total)
      |> Telemetry.int_ ~key:"hwm_bytes" ~value:(total_hwm - initial.total)
      |> Telemetry.int_opt
           ~key:"relative_to_initial"
           ~value:(Option.some_if initial_opt initial.total)
      |> Telemetry.int_opt
           ~key:"sysinfo_freeram"
           ~value:(Option.map sysinfo ~f:(fun si -> si.Sys_utils.freeram))
      |> Telemetry.int_opt
           ~key:"sysinfo_freeswap"
           ~value:(Option.map sysinfo ~f:(fun si -> si.Sys_utils.freeswap))
      |> Telemetry.int_opt
           ~key:"sysinfo_totalswap"
           ~value:(Option.map sysinfo ~f:(fun si -> si.Sys_utils.totalswap))
      |> Option.some;
    HackEventLogger.CGroup.step
      ~cgroup:cgroup.cgroup_name
      ~step_group:step_group.name
      ~step
      ~start_time
      ~total_relative_to:(Option.some_if initial_opt initial.total)
      ~anon_relative_to:(Option.some_if initial_opt initial.anon)
      ~shmem_relative_to:(Option.some_if initial_opt initial.shmem)
      ~file_relative_to:(Option.some_if initial_opt initial.file)
      ~total_start:(Some (start_cgroup.total - initial.total))
      ~anon_start:(Some (start_cgroup.anon - initial.anon))
      ~shmem_start:(Some (start_cgroup.shmem - initial.shmem))
      ~file_start:(Some (start_cgroup.file - initial.file))
      ~total_hwm:(total_hwm - initial.total)
      ~total:(cgroup.total - initial.total)
      ~anon:(cgroup.anon - initial.anon)
      ~shmem:(cgroup.shmem - initial.shmem)
      ~file:(cgroup.file - initial.file)
      ~secs_at_total_gb:(Some (secs_at_total_gb |> Array.to_list))
      ~secs_above_total_gb_summary
  | (None, Ok cgroup) ->
    telemetry_ref :=
      Telemetry.create ()
      |> Telemetry.int_ ~key:"end" ~value:cgroup.total
      |> Option.some;
    HackEventLogger.CGroup.step
      ~cgroup:cgroup.cgroup_name
      ~step_group:step_group.name
      ~step
      ~start_time:None
      ~total_relative_to:(Option.some_if initial_opt initial.total)
      ~anon_relative_to:(Option.some_if initial_opt initial.anon)
      ~shmem_relative_to:(Option.some_if initial_opt initial.shmem)
      ~file_relative_to:(Option.some_if initial_opt initial.file)
      ~total_start:None
      ~anon_start:None
      ~shmem_start:None
      ~file_start:None
      ~total_hwm:(cgroup.total - initial.total)
      ~total:(cgroup.total - initial.total)
      ~anon:(cgroup.anon - initial.anon)
      ~shmem:(cgroup.shmem - initial.shmem)
      ~file:(cgroup.file - initial.file)
      ~secs_at_total_gb:None
      ~secs_above_total_gb_summary:SMap.empty

let step_group name ~log f =
  let log = log && Sys_utils.enable_telemetry () in
  let profiling =
    {
      name;
      log;
      step_count = ref 0;
      last_printed_total = ref 0;
      prev_step_to_show = ref None;
    }
  in
  f profiling

let step step_group ?(telemetry_ref = ref None) name =
  if not step_group.log then
    ()
  else begin
    step_group.step_count := !(step_group.step_count) + 1;
    let step = Printf.sprintf "%02d_%s" !(step_group.step_count) name in
    let cgroup = CGroup.get_stats () in
    log_cgroup_total cgroup ~step_group ~step ~suffix:"" ~total_hwm:0;
    log_telemetry
      ~step_group
      ~step
      ~start_time:None
      ~total_hwm:0
      ~start_cgroup:None
      ~cgroup
      ~telemetry_ref
      ~secs_at_total_gb:[||]
  end

let step_start_end step_group ?(telemetry_ref = ref None) name f =
  if not step_group.log then
    f ()
  else begin
    step_group.step_count := !(step_group.step_count) + 1;
    let step = Printf.sprintf "%02d_%s" !(step_group.step_count) name in
    let start_time = Unix.gettimeofday () in
    let start_cgroup = CGroup.get_stats () in
    log_cgroup_total
      start_cgroup
      ~step_group
      ~step
      ~suffix:" start"
      ~total_hwm:0;
    let (initial, ()) =
      initial_value_map start_cgroup ~default:() ~f:(fun _ -> ())
    in
    Result.iter start_cgroup ~f:(fun { CGroup.cgroup_name; _ } ->
        let path1 = "/sys/fs/cgroup/" ^ cgroup_name ^ "/memory.current" in
        let path2 = "/sys/fs/cgroup/" ^ cgroup_name ^ "/memory.swap.current" in
        cgroup_watcher_start
          path1
          path2
          ~subtract_kb_for_array:(initial.CGroup.total / 1024));
    Utils.try_finally ~f ~finally:(fun () ->
        try
          let cgroup = CGroup.get_stats () in
          let (hwm_kb, _num_readings, secs_at_total_gb) =
            cgroup_watcher_get ()
          in
          let total_hwm = hwm_kb * 1024 in
          log_cgroup_total cgroup ~step_group ~step ~suffix:" end" ~total_hwm;
          log_telemetry
            ~step_group
            ~step
            ~start_time:(Some start_time)
            ~total_hwm
            ~start_cgroup:(Some start_cgroup)
            ~cgroup
            ~telemetry_ref
            ~secs_at_total_gb
        with
        | exn ->
          let e = Exception.wrap exn in
          telemetry_ref :=
            Telemetry.create () |> Telemetry.exception_ ~e |> Option.some;
          Hh_logger.log "cgroup - failed to log. %s" (Exception.to_string e))
  end

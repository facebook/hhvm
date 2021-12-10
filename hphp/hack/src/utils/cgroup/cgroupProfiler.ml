open Hh_prelude

external cgroup_watcher_start : string -> unit = "cgroup_watcher_start"

external cgroup_watcher_get : unit -> int * int * float array
  = "cgroup_watcher_get"

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
let log_cgroup_total cgroup ~step_group ~step ~suffix ~hwm =
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
        if cgroup.CGroup.total >= hwm then
          ""
        else
          Printf.sprintf " (hwm %s)" (pretty_num hwm)
      in
      let prev =
        match prev with
        | None -> ""
        | Some prev ->
          Printf.sprintf
            "\n   >%s %s  [@%s]  previous measurement"
            (Utils.timestring prev.time)
            (pretty_num prev.step_total)
            prev.log_label_and_suffix
      in
      Hh_logger.log
        "Cgroup: %s%s  [@%s]%s"
        (pretty_num cgroup.CGroup.total)
        hwm
        current.log_label_and_suffix
        prev
    end

(** Given a float array [gbs] where gbs[0] says how many secs were spent at cgroup memory 0gb,
gbs[1] says how many secs were spent at cgroup memory 1gb, and so on, produces an SMap
where keys are some arbitrary thresholds "secs_above_20gb" and values are (int) number of seconds
spent at that memory or higher. We pick just a few arbitrary thresholds that we think are useful
for telemetry, and their sole purpose is telemetry. *)
let secs_above (gbs : float array) : int SMap.t =
  if Array.is_empty gbs then
    SMap.empty
  else
    List.init 27 ~f:(fun i -> i * 5) (* 0gb, 5gb, ..., 130gb) *)
    |> List.map ~f:(fun threshold ->
           let title = Printf.sprintf "secs_above_%dgb" threshold in
           let secs =
             Array.foldi gbs ~init:0. ~f:(fun i acc s ->
                 acc
                 +.
                 if i < threshold then
                   0.
                 else
                   s)
           in
           (title, Float.round secs |> int_of_float))
    |> SMap.of_list

(** Records to HackEventLogger *)
let log_telemetry
    ~step_group ~step ~start_time ~hwm ~start_cgroup ~cgroup ~telemetry_ref ~gbs
    =
  let open CGroup in
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
    let total_hwm = max (max start_cgroup.total cgroup.total) hwm in
    let secs_above = secs_above gbs in
    telemetry_ref :=
      Telemetry.create ()
      |> SMap.fold (fun key value -> Telemetry.int_ ~key ~value) secs_above
      |> Telemetry.int_ ~key:"start_bytes" ~value:start_cgroup.total
      |> Telemetry.int_ ~key:"end_bytes" ~value:cgroup.total
      |> Telemetry.int_ ~key:"hwm_bytes" ~value:total_hwm
      |> Option.some;
    HackEventLogger.CGroup.step
      ~cgroup:cgroup.cgroup_name
      ~step_group:step_group.name
      ~step
      ~start_time
      ~total_start:(Some start_cgroup.total)
      ~totalswap_start:(Some start_cgroup.total_swap)
      ~anon_start:(Some start_cgroup.anon)
      ~shmem_start:(Some start_cgroup.shmem)
      ~file_start:(Some start_cgroup.file)
      ~total_hwm
      ~total:cgroup.total
      ~totalswap:cgroup.total_swap
      ~anon:cgroup.anon
      ~shmem:cgroup.shmem
      ~file:cgroup.file
      ~gbs:(Some (gbs |> Array.to_list))
      ~secs_above
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
      ~total_start:None
      ~totalswap_start:None
      ~anon_start:None
      ~shmem_start:None
      ~file_start:None
      ~total_hwm:cgroup.total
      ~total:cgroup.total
      ~totalswap:cgroup.total_swap
      ~anon:cgroup.anon
      ~shmem:cgroup.shmem
      ~file:cgroup.file
      ~gbs:None
      ~secs_above:SMap.empty

let step_group name ~log f =
  let log = log && not (Sys_utils.is_test_mode ()) in
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
    log_cgroup_total cgroup ~step_group ~step ~suffix:"" ~hwm:0;
    log_telemetry
      ~step_group
      ~step
      ~start_time:None
      ~hwm:0
      ~start_cgroup:None
      ~cgroup
      ~telemetry_ref
      ~gbs:[||]
  end

let step_start_end step_group ?(telemetry_ref = ref None) name f =
  if not step_group.log then
    f ()
  else begin
    step_group.step_count := !(step_group.step_count) + 1;
    let step = Printf.sprintf "%02d_%s" !(step_group.step_count) name in
    let start_time = Unix.gettimeofday () in
    let start_cgroup = CGroup.get_stats () in
    log_cgroup_total start_cgroup ~step_group ~step ~suffix:" start" ~hwm:0;
    Result.iter start_cgroup ~f:(fun { CGroup.cgroup_name; _ } ->
        let path = "/sys/fs/cgroup/" ^ cgroup_name ^ "/memory.current" in
        cgroup_watcher_start path);
    Utils.try_finally ~f ~finally:(fun () ->
        try
          let cgroup = CGroup.get_stats () in
          let (hwm_kb, _num_readings, gbs) = cgroup_watcher_get () in
          let hwm = hwm_kb * 1024 in
          log_cgroup_total cgroup ~step_group ~step ~suffix:" end" ~hwm;
          log_telemetry
            ~step_group
            ~step
            ~start_time:(Some start_time)
            ~hwm
            ~start_cgroup:(Some start_cgroup)
            ~cgroup
            ~telemetry_ref
            ~gbs
        with
        | exn ->
          let e = Exception.wrap exn in
          telemetry_ref :=
            Telemetry.create () |> Telemetry.exception_ ~e |> Option.some;
          Hh_logger.log "cgroup - failed to log. %s" (Exception.to_string e))
  end

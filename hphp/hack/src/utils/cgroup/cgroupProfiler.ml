open Hh_prelude

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

type step = { total_hwm_ref: int ref }

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

(** Records to HackEventLogger *)
let log_telemetry ~step_group ~step ~start_time ~hwm ~start_cgroup ~cgroup =
  let open CGroup in
  match (start_cgroup, cgroup) with
  | (Some (Error s), _)
  | (_, Error s) ->
    if SSet.mem s !has_logged_error then
      ()
    else begin
      has_logged_error := SSet.add s !has_logged_error;
      HackEventLogger.CGroup.error s
    end
  | (Some (Ok start_cgroup), Ok cgroup) ->
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
      ~total_hwm:(max cgroup.total hwm)
      ~total:cgroup.total
      ~totalswap:cgroup.total_swap
      ~anon:cgroup.anon
      ~shmem:cgroup.shmem
      ~file:cgroup.file
  | (None, Ok cgroup) ->
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

let step step_group name =
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
  end

let step_start_end step_group name f =
  if not step_group.log then
    f { total_hwm_ref = ref 0 }
  else begin
    step_group.step_count := !(step_group.step_count) + 1;
    let step = Printf.sprintf "%02d_%s" !(step_group.step_count) name in
    let start_time = Unix.gettimeofday () in
    let start_cgroup = CGroup.get_stats () in
    log_cgroup_total start_cgroup ~step_group ~step ~suffix:" start" ~hwm:0;
    let hwm = { total_hwm_ref = ref 0 } in
    Utils.try_finally
      ~f:(fun () -> f hwm)
      ~finally:(fun () ->
        try
          let cgroup = CGroup.get_stats () in
          log_cgroup_total
            cgroup
            ~step_group
            ~step
            ~suffix:" end"
            ~hwm:!(hwm.total_hwm_ref);
          log_telemetry
            ~step_group
            ~step
            ~start_time:(Some start_time)
            ~hwm:!(hwm.total_hwm_ref)
            ~start_cgroup:(Some start_cgroup)
            ~cgroup
        with
        | exn ->
          let e = Exception.wrap exn in
          Hh_logger.log "cgroup - failed to log. %s" (Exception.to_string e))
  end

let update_cgroup_total total { total_hwm_ref } =
  total_hwm_ref := max !total_hwm_ref total;
  ()

open Hh_prelude

module Profiling = struct
  type values = {
    (* memory at the start of a stage *)
    start: float;
    (* memory at the end of a stage *)
    end_: float;
    (* the highest memory recorded during a stage *)
    high_water_mark: float;
  }

  (* mapping from metrics to their corresponding values *)
  and result = values SMap.t

  and profiling = {
    (* the event that is being profiled, e.g. Init, Recheck, etc. *)
    event: string;
    (* should we log stages? *)
    log: bool;
    (* the various stages of an event in reverse order *)
    stages_rev: string list;
    (* stage mapped to result *)
    results: result SMap.t;
  }

  and t = profiling ref

  let get_stage_result_map ~(stage : string) (profiling : t) : result =
    match SMap.find_opt stage !profiling.results with
    | None ->
      profiling :=
        {
          !profiling with
          stages_rev = stage :: !profiling.stages_rev;
          results = SMap.add stage SMap.empty !profiling.results;
        };
      SMap.empty
    | Some stage_result -> stage_result

  let get_metric ~(stage : string) ~(metric : string) (profiling : t) :
      values option =
    get_stage_result_map ~stage profiling |> SMap.find_opt metric

  let set_metric
      ~(stage : string) ~(metric : string) (values : values) (profiling : t) :
      unit =
    let stage_results =
      get_stage_result_map ~stage profiling |> SMap.add metric values
    in
    profiling :=
      {
        !profiling with
        results = SMap.add stage stage_results !profiling.results;
      }

  let start_sampling
      ~(stage : string) ~(metric : string) ~(value : float) (profiling : t) :
      unit =
    let new_metric = { start = value; end_ = value; high_water_mark = value } in
    set_metric ~stage ~metric new_metric profiling

  let record_stats
      ~(stage : string) ~(metric : string) ~(value : float) ~(profiling : t) :
      unit =
    match get_metric ~stage ~metric profiling with
    | None -> start_sampling ~stage ~metric ~value profiling
    | Some old_metric ->
      let new_metric =
        {
          old_metric with
          end_ = value;
          high_water_mark = Float.max value old_metric.high_water_mark;
        }
      in
      set_metric ~stage ~metric new_metric profiling
end

let sample_cgroup_mem ~(profiling : Profiling.t) ~(stage : string) : unit =
  let cgroup_stats = CGroup.get_stats () in
  match cgroup_stats with
  | Error _ -> ()
  | Ok { CGroup.total; total_swap; anon; file; shmem } ->
    let is_end =
      Profiling.get_stage_result_map ~stage profiling |> SMap.mem "cgroup_total"
    in
    Profiling.record_stats
      ~profiling
      ~stage
      ~metric:"cgroup_total"
      ~value:(float total);
    Profiling.record_stats
      ~profiling
      ~stage
      ~metric:"cgroup_swap"
      ~value:(float total_swap);
    Profiling.record_stats
      ~profiling
      ~stage
      ~metric:"cgroup_anon"
      ~value:(float anon);
    Profiling.record_stats
      ~profiling
      ~stage
      ~metric:"cgroup_shmem"
      ~value:(float shmem);
    Profiling.record_stats
      ~profiling
      ~stage
      ~metric:"cgroup_file"
      ~value:(float file);
    let open Profiling in
    if !profiling.log then begin
      let pretty_num f = Printf.sprintf "%0.2fGiB" (f /. 1073741824.0) in
      let result = SMap.find stage !profiling.results in
      let values = SMap.find "cgroup_total" result in
      if is_end then
        let hwm =
          if Float.( < ) values.end_ values.high_water_mark then
            Printf.sprintf
              "  --  high water mark %s"
              (pretty_num values.high_water_mark)
          else
            ""
        in
        Hh_logger.log
          "Cgroup: %s  [%s/%s end]%s"
          (pretty_num values.end_)
          !profiling.event
          stage
          hwm
      else
        Hh_logger.log
          "Cgroup: %s  [%s/%s...]"
          (pretty_num values.start)
          !profiling.event
          stage;
      ()
    end

type event = Profiling.t

type stage = {
  profiling: Profiling.t;
  stage: string;
}

let event ~event ~log f =
  let log = log && not (Sys_utils.is_test_mode ()) in
  let profiling =
    ref Profiling.{ event; log; stages_rev = []; results = SMap.empty }
  in
  f profiling

let stage profiling ~stage f =
  let i = 1 + List.length !profiling.Profiling.stages_rev in
  let stage = Printf.sprintf "%02d_%s" i stage in
  (* sample memory stats before running f *)
  sample_cgroup_mem ~profiling ~stage;
  Utils.try_finally
    ~f:(fun () -> f { profiling; stage })
    ~finally:(fun () ->
      try
        (* sample memory stats after running f *)
        sample_cgroup_mem ~profiling ~stage;
        (* log the recorded stats to scuba as well *)
        match SMap.find_opt stage !profiling.Profiling.results with
        | None -> ()
        | Some result ->
          let cgroup =
            match ProcFS.first_cgroup_for_pid (Unix.getpid ()) with
            | Ok cgroup -> cgroup
            | Error err -> Printf.sprintf "Error getting cgroup: %s" err
          in
          let event = !profiling.Profiling.event in
          SMap.iter
            (fun metric value ->
              HackEventLogger.CGroup.profile
                ~cgroup
                ~event
                ~stage
                ~metric
                ~start:value.Profiling.start
                ~end_:value.Profiling.end_
                ~hwm:value.Profiling.high_water_mark)
            result
      with
      | exn ->
        let e = Exception.wrap exn in
        Hh_logger.log "cgroup - failed to log. %s" (Exception.to_string e))

let update_cgroup_total value { profiling; stage } =
  Profiling.record_stats ~stage ~metric:"cgroup_total" ~value ~profiling

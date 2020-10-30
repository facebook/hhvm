module Profiling = struct
  type values = {
    start: float;
    delta: float;
    high_water_mark_delta: float;
  }

  and result = values SMap.t

  and profiling = {
    event: string;
    stages_rev: string list;
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
    let new_metric =
      { start = value; delta = 0.0; high_water_mark_delta = 0.0 }
    in
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
          delta = value -. old_metric.start;
          high_water_mark_delta =
            max (value -. old_metric.start) old_metric.high_water_mark_delta;
        }
      in
      set_metric ~stage ~metric new_metric profiling

  let print_summary_memory_table =
    let pretty_num f =
      let abs_f = abs_float f in
      if abs_f > 1000000000.0 then
        Printf.sprintf "%+7.2fG" (f /. 1000000000.0)
      else if abs_f > 1000000.0 then
        Printf.sprintf "%+7.2fM" (f /. 1000000.0)
      else if abs_f > 1000.0 then
        Printf.sprintf "%+7.2fK" (f /. 1000.0)
      else
        Printf.sprintf "%+7.2f " f
    in
    let pretty_pct num denom =
      if denom = 0.0 then
        "(--N/A--)"
      else
        let fraction = num /. denom in
        if
          fraction >= 10.0
          (* e.g "( +20.4x)" fits the space whereas (+2040.0%) doesn't *)
        then
          Printf.sprintf "(%+6.1fx)" fraction
        else
          Printf.sprintf "(%+6.1f%%)" (fraction *. 100.0)
    in
    (* Prints a single row of the table. All but the last column have a fixed width. *)
    let print_summary_single ~indent key result =
      let indent = String.make indent ' ' in
      Printf.eprintf
        "%s        %s %s    %s %s    %s%s\n%!"
        (pretty_num result.start)
        (pretty_num result.delta)
        (pretty_pct result.delta result.start)
        (pretty_num result.high_water_mark_delta)
        (pretty_pct result.high_water_mark_delta result.start)
        indent
        key
    in
    let header_without_section =
      "  START                DELTA               HWM DELTA          "
    in
    let pre_section_whitespace =
      String.make (String.length header_without_section) ' '
    in
    let print_group ~indent finished_results group_name =
      Base.Option.iter
        (SMap.find_opt group_name finished_results)
        ~f:(fun group ->
          let indent_str =
            String.make (String.length header_without_section + indent - 2) ' '
          in
          Printf.eprintf "%s== %s ==\n%!" indent_str group_name;
          SMap.iter (print_summary_single ~indent:(indent + 2)) group)
    in
    let print_header label =
      let label =
        Printf.sprintf "%s Memory Stats" (String.uppercase_ascii label)
      in
      let header = header_without_section ^ "SECTION" in
      let header_len = String.length header + 8 in
      let whitespace_len = header_len - String.length label in
      Printf.eprintf
        "%s%s%s\n%!"
        (String.make ((whitespace_len + 1) / 2) '=')
        label
        (String.make (whitespace_len / 2) '=');
      Printf.eprintf "%s\n%!" header;
      Printf.eprintf "%s\n%!" (String.make header_len '-')
    in
    let print_finished ~indent results =
      if not (SMap.is_empty !results.results) then (
        let header_indent = String.make indent '=' in
        Printf.eprintf
          "%s%s %s %s\n%!"
          pre_section_whitespace
          header_indent
          !results.event
          header_indent;
        let indent = indent + 2 in
        List.iter
          (print_group ~indent !results.results)
          (List.rev !results.stages_rev)
      )
    in
    fun memory ->
      if SMap.cardinal !memory.results > 0 then (
        print_header !memory.event;
        print_finished ~indent:2 memory
      )
end

let sample_cgroup_mem ~(profiling : Profiling.t) ~(stage : string) : unit =
  let cgroup_stats = CGroup.get_stats () in
  match cgroup_stats with
  | Error _ -> ()
  | Ok { CGroup.total; total_swap; anon; file; shmem } ->
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
      ~value:(float file)

let collect_cgroup_stats ~profiling ~stage ~f =
  sample_cgroup_mem ~profiling ~stage;
  let ret = f () in
  sample_cgroup_mem ~profiling ~stage;
  ret

let profile_memory ~event ~f =
  let profiling =
    ref Profiling.{ event; stages_rev = []; results = SMap.empty }
  in
  let ret = f profiling in
  (profiling, ret)

let print_summary_memory_table = Profiling.print_summary_memory_table

let log_result_to_scuba
    ~(event : string) ~(stage : string) (result : Profiling.result) : unit =
  SMap.iter
    (fun metric value ->
      HackEventLogger.CGroup.profile
        ~event
        ~stage
        ~metric
        ~start:value.Profiling.start
        ~delta:value.Profiling.delta
        ~hwm_delta:value.Profiling.high_water_mark_delta)
    result

let log_to_scuba ~(stage : string) ~(profiling : Profiling.t) : unit =
  match SMap.find_opt stage !profiling.Profiling.results with
  | None -> ()
  | Some result ->
    log_result_to_scuba ~event:!profiling.Profiling.event ~stage result

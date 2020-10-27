module MemStats = struct
  type memory_result = {
    start: float;
    delta: float;
    high_water_mark_delta: float;
  }

  and running' = {
    running_groups_rev: string list;
    running_results: memory_result SMap.t SMap.t;
    running_sub_results_rev: finished list;
  }

  and running = running' ref

  and finished = {
    finished_label: string;
    finished_groups: string list;
    finished_results: memory_result SMap.t SMap.t;
    finished_sub_results: finished list;
  }

  let get_group_map ~group running_memory =
    match SMap.find_opt group !running_memory.running_results with
    | None ->
      running_memory :=
        {
          !running_memory with
          running_groups_rev = group :: !running_memory.running_groups_rev;
          running_results =
            SMap.add group SMap.empty !running_memory.running_results;
        };
      SMap.empty
    | Some group -> group

  let get_metric ~group ~metric running_memory =
    get_group_map ~group running_memory |> SMap.find_opt metric

  let set_metric ~group ~metric entry running_memory =
    let group_map =
      get_group_map ~group running_memory |> SMap.add metric entry
    in
    running_memory :=
      {
        !running_memory with
        running_results =
          SMap.add group group_map !running_memory.running_results;
      }

  let start_sampling ~group ~metric ~value running_memory =
    let new_metric =
      { start = value; delta = 0.0; high_water_mark_delta = 0.0 }
    in
    set_metric ~group ~metric new_metric running_memory

  let sample_memory ~group ~metric ~value running_memory =
    match get_metric ~group ~metric running_memory with
    | None -> start_sampling ~group ~metric ~value running_memory
    | Some old_metric ->
      let new_metric =
        {
          old_metric with
          delta = value -. old_metric.start;
          high_water_mark_delta =
            max (value -. old_metric.start) old_metric.high_water_mark_delta;
        }
      in
      set_metric ~group ~metric new_metric running_memory

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
      let label = Printf.sprintf "%s Memory Stats" label in
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
    let rec print_finished ~indent results =
      if
        (not (SMap.is_empty results.finished_results))
        || results.finished_sub_results <> []
      then (
        let header_indent = String.make indent '=' in
        Printf.eprintf
          "%s%s %s %s\n%!"
          pre_section_whitespace
          header_indent
          results.finished_label
          header_indent;
        let indent = indent + 2 in
        List.iter
          (print_group ~indent results.finished_results)
          results.finished_groups;
        List.iter
          (fun sub_result -> print_finished ~indent sub_result)
          results.finished_sub_results
      )
    in
    fun memory ->
      if
        SMap.cardinal memory.finished_results > 0
        || memory.finished_sub_results <> []
      then (
        print_header memory.finished_label;
        print_finished ~indent:2 memory
      )
end

let sample_cgroup_mem group mem_stats =
  let cgroup_stats = CGroup.get_stats () in
  match cgroup_stats with
  | Error _ -> ()
  | Ok { CGroup.total; total_swap; anon; file; shmem } ->
    MemStats.sample_memory
      mem_stats
      ~group
      ~metric:"cgroup_total"
      ~value:(float total);
    MemStats.sample_memory
      mem_stats
      ~group
      ~metric:"cgroup_swap"
      ~value:(float total_swap);
    MemStats.sample_memory
      mem_stats
      ~group
      ~metric:"cgroup_anon"
      ~value:(float anon);
    MemStats.sample_memory
      mem_stats
      ~group
      ~metric:"cgroup_shmem"
      ~value:(float shmem);
    MemStats.sample_memory
      mem_stats
      ~group
      ~metric:"cgroup_file"
      ~value:(float file)

let collect_cgroup_stats mem_stats ~group ~f =
  sample_cgroup_mem group mem_stats;
  let ret = f () in
  sample_cgroup_mem group mem_stats;
  ret

let profile_memory ~label ~f =
  let running_memory =
    ref
      MemStats.
        {
          running_groups_rev = [];
          running_results = SMap.empty;
          running_sub_results_rev = [];
        }
  in
  let ret = f running_memory in
  let finished_memory =
    MemStats.
      {
        finished_label = label;
        finished_groups = List.rev !running_memory.running_groups_rev;
        finished_results = !running_memory.running_results;
        finished_sub_results = List.rev !running_memory.running_sub_results_rev;
      }
  in
  (finished_memory, ret)

let print_summary_memory_table = MemStats.print_summary_memory_table

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type env = {
  root: Path.t;
  from: string;
  rageid: string option;
  desc: string;
}

let format_failure (message : string) (failure : Lwt_utils.Process_failure.t) :
    string =
  let open Lwt_utils.Process_failure in
  let status =
    match failure.process_status with
    | Unix.WEXITED i ->
      Printf.sprintf "WEXITED %d (%s)" i (Exit_status.exit_code_to_string i)
    | Unix.WSIGNALED i ->
      let msg =
        if i = Sys.sigkill then
          " this signal might have been sent by 'hh rage' itself if the command took too long"
        else
          ""
      in
      Printf.sprintf
        "WSIGNALED %d (%s)%s"
        i
        (PrintSignal.string_of_signal i)
        msg
    | Unix.WSTOPPED i ->
      Printf.sprintf "WSTOPPED %d (%s)" i (PrintSignal.string_of_signal i)
  in
  Printf.sprintf
    "%s\nCMDLINE: %s\nEXIT STATUS: %s\nSTART: %s\nEND: %s\n\nSTDOUT:\n%s\nSTDERR:\n%s\n"
    message
    failure.command_line
    status
    (Utils.timestring failure.start_time)
    (Utils.timestring failure.end_time)
    failure.stdout
    failure.stderr

let get_stack (pid, reason) : string Lwt.t =
  let pid = string_of_int pid in
  let format msg = Printf.sprintf "PSTACK %s (%s)\n%s\n" pid reason msg in
  match%lwt
    Lwt_utils.exec_checked Exec_command.Pstack [| pid |] ~timeout:60.0
  with
  | Ok result ->
    let stack = result.Lwt_utils.Process_success.stdout in
    Lwt.return (format stack)
  | Error _ ->
    (* pstack is just an alias for gstack, but it's not present on all systems. *)
    (match%lwt
       Lwt_utils.exec_checked Exec_command.Gstack [| pid |] ~timeout:60.0
     with
    | Ok result ->
      let stack = result.Lwt_utils.Process_success.stdout in
      Lwt.return (format stack)
    | Error e ->
      let err = "unable to get stack: " ^ e.Lwt_utils.Process_failure.stderr in
      Lwt.return (format err))

let pgrep pattern =
  let%lwt result =
    Lwt_utils.exec_checked ~timeout:20.0 Exec_command.Pgrep [| "-a"; pattern |]
  in
  match result with
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    let re = Str.regexp {|^\([0-9]+\) \(.*\)$|} in
    let pids =
      String.split_lines stdout
      |> List.filter_map ~f:(fun s ->
             try
               if Str.string_match re s 0 then
                 let pid = Str.matched_group 1 s |> int_of_string in
                 let reason = Str.matched_group 2 s in
                 Some (pid, reason)
               else
                 None
             with _ -> None)
    in
    Lwt.return pids
  | Error _ -> Lwt.return []

let rage_strobelight () : string Lwt.t =
  (* We'll run strobelight on each hh_server ServerMain process *)
  let%lwt pids = pgrep "hh_server" in
  let pids =
    List.filter_map pids ~f:(fun (pid, reason) ->
        Option.some_if
          (String_utils.is_substring "ServerMain" reason)
          (Some (pid, reason)))
  in
  (* We'll also run strobelight without specifying a pid *)
  let pids = None :: pids in
  (* Strobelight doesn't support parallel execution, so we do it in sequence *)
  let%lwt results =
    Lwt_list.map_s
      (fun pid ->
        let common_args =
          [
            "run";
            "--profile";
            "fastbpf";
            "--event";
            "cpu-clock";
            "--sample-rate";
            "10000000";
            "--duration-ms";
            "10000";
          ]
        in
        let (pid_args, reason) =
          match pid with
          | None -> ([], "ALL PROCESSES")
          | Some (pid, reason) ->
            ( ["--pid"; string_of_int pid],
              Printf.sprintf "PROCESS %d - %s" pid reason )
        in
        let%lwt result =
          Lwt_utils.exec_checked
            ~timeout:30.0
            Exec_command.Strobeclient
            (common_args @ pid_args |> Array.of_list)
        in
        let result =
          match result with
          | Ok { Lwt_utils.Process_success.stdout; stderr; _ } ->
            reason ^ "\n" ^ stdout ^ "\n" ^ stderr
          | Error failure -> reason ^ format_failure "" failure
        in
        Lwt.return result)
      pids
  in
  let results = String.concat results ~sep:"\n\n\n" in
  Lwt.return results

let rage_pstacks (env : env) : string Lwt.t =
  (* We'll look at all relevant pids: all those from the hh_server
  binary, and the hh_client binary, and all those in the server pids_file.
  We put them into a map from pid to "reason" so that each relevant
  pid is only picked once. The "reason" is a useful string description:
  pgrep shows the cmdline that spawned a given pid so we use that as
  a reason; the server pids_file also stores reasons. *)
  let%lwt hh_server_pids = pgrep "hh_server" in
  let%lwt hh_client_pids = pgrep "hh_client" in
  let server_pids =
    (try PidLog.get_pids (ServerFiles.pids_file env.root) with _ -> [])
  in
  let pids = IMap.empty in
  let pids =
    List.fold hh_server_pids ~init:pids ~f:(fun acc (pid, reason) ->
        IMap.add pid reason acc)
  in
  let pids =
    List.fold hh_client_pids ~init:pids ~f:(fun acc (pid, reason) ->
        IMap.add pid reason acc)
  in
  let pids =
    List.fold server_pids ~init:pids ~f:(fun acc (pid, reason) ->
        IMap.add pid reason acc)
  in
  let pids = IMap.bindings pids in
  (* Pstacks take a while to collect and some are uninteresting.
  We'll filter out all Scuba, and all but one worker subprocess. Keep just
  one worker in case the workers are stuck for some reason. *)
  let (pids, _) =
    List.fold
      pids
      ~init:([], false)
      ~f:(fun (acc, has_subprocess) (pid, reason) ->
        if String_utils.is_substring "scuba for process" reason then
          (acc, has_subprocess)
        else if String_utils.string_starts_with reason "subprocess" then
          if has_subprocess then
            (acc, has_subprocess)
          else
            ((pid, reason) :: acc, true)
        else
          ((pid, reason) :: acc, has_subprocess))
  in
  (* I don't know why pstacks are slow; I don't know what their
  bottleneck is. But I observed that doing them in parallel didn't hurt. *)
  let%lwt stacks = Lwt_list.map_p get_stack pids in
  let stacks = String.concat stacks ~sep:"\n\n" in
  Lwt.return stacks

let rage_ps () : string Lwt.t =
  (* Flags to ps:
  -A means "all processes"
  -F means "extra full output" i.e. lots of fields of output.
  --forest means "ascii art process tree" *)
  let%lwt result =
    Lwt_utils.exec_checked ~timeout:20.0 Exec_command.Ps [| "-AF"; "--forest" |]
  in
  match result with
  | Ok { Lwt_utils.Process_success.stdout; _ } -> Lwt.return stdout
  | Error failure -> Lwt.return (format_failure "" failure)

let rage_hh_version
    (env : env) (hhconfig_version_raw : Config_file.version option) :
    string Lwt.t =
  let version =
    Option.bind
      hhconfig_version_raw
      ~f:(Config_file.version_to_string_opt ~pad:false)
  in
  let hhconfig_update =
    match version with
    | None -> ""
    | Some version ->
      Printf.sprintf
        "hg pull -B releases/hack/v%s\nhg update -C remote/releases/hack/v%s"
        version
        version
  in
  let hh_home_env =
    Sys.getenv_opt "HH_HOME" |> Option.value ~default:"[unset]"
  in
  let hack_rc_mode =
    Sys_utils.expanduser "~/.hack_rc_mode"
    |> Sys_utils.cat_or_failed
    |> Option.value ~default:"[absent]"
  in
  let%lwt hh_server_version_result =
    Lwt_utils.exec_checked
      ~timeout:20.0
      (Exec_command.Hh_server "hh_server")
      [| "--version"; Path.to_string env.root |]
  in
  let hh_server_version =
    match hh_server_version_result with
    | Ok { Lwt_utils.Process_success.stdout; _ } -> stdout
    | Error failure -> format_failure "" failure
  in
  let hh_version =
    Printf.sprintf
      ( "build_commit_time: %d (%s)\n"
      ^^ "build_mode: %s\n"
      ^^ "build_revision: %s\n"
      ^^ "hhconfig_version: %s\n"
      ^^ "$HH_HOME: %s\n"
      ^^ "~/.hack_rc_mode: %s\n"
      ^^ "executable_name: %s\n"
      ^^ "\nhh_server --version: %s\n"
      ^^ "\n%s" )
      Build_id.build_commit_time
      Build_id.build_commit_time_string
      Build_id.build_mode
      ( if String.equal Build_id.build_revision "" then
        "[empty]"
      else
        Build_id.build_revision )
      (Option.value version ~default:"[absent]")
      hh_home_env
      hack_rc_mode
      Sys.executable_name
      hh_server_version
      hhconfig_update
  in
  Lwt.return hh_version

let rage_hh_server_state (env : env) :
    ((string * string) list, string) result Lwt.t =
  let open Hh_json in
  let json_item_to_pair json_item =
    match json_item with
    | JSON_Object
        [("name", JSON_String name); ("contents", JSON_String contents)]
    | JSON_Object
        [("contents", JSON_String contents); ("name", JSON_String name)] ->
      ("hh_server_" ^ name, contents)
    | _ -> raise (Syntax_error "unexpected item; expected {name:_, contents:_}")
  in
  let%lwt hh_server_state_result =
    Lwt_utils.exec_checked
      ~timeout:20.0
      Exec_command.Hh
      [|
        "check";
        "--server-rage";
        "--autostart-server";
        "false";
        "--from";
        "rage";
        "--json";
        Path.to_string env.root;
      |]
  in
  match hh_server_state_result with
  | Error failure ->
    Lwt.return_error (format_failure "failed to obtain state" failure)
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    begin
      try
        match json_of_string stdout with
        | JSON_Array json_items ->
          Lwt.return_ok (List.map json_items ~f:json_item_to_pair)
        | _ -> raise (Syntax_error "unexpected json; expected array")
      with Syntax_error msg ->
        Lwt.return_error
          (Printf.sprintf "unable to parse json: %s\n\n%s\n" msg stdout)
    end

let rage_www (env : env) : ((string * string) option * string) Lwt.t =
  let hgplain_env =
    Process.env_to_array (Process_types.Augment ["HGPLAIN=1"])
  in
  let%lwt www_result =
    Lwt_utils.exec_checked
      ?env:hgplain_env
      ~timeout:60.0
      Exec_command.Hg
      [|
        "log";
        "-r";
        "last(public() & :: .)";
        "-T";
        "{node}";
        "--cwd";
        Path.to_string env.root;
      |]
  in
  match www_result with
  | Error failure ->
    Lwt.return (None, format_failure "Unable to determine mergebase" failure)
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    let mergebase = stdout in
    let%lwt www_diff_result =
      Lwt_utils.exec_checked
        ?env:hgplain_env
        Exec_command.Hg
        ~timeout:60.0
        [| "diff"; "-r"; mergebase; "--cwd"; Path.to_string env.root |]
    in
    let%lwt (patch_item, patch_instructions) =
      match www_diff_result with
      | Error failure ->
        Lwt.return (None, format_failure "Unable to determine diff" failure)
      | Ok { Lwt_utils.Process_success.stdout = hgdiff; _ } ->
        if String.is_empty hgdiff then
          Lwt.return (None, "")
        else
          let%lwt clowder_result =
            Clowder_paste.clowder_upload_and_get_shellscript
              ~timeout:60.0
              hgdiff
          in
          (match clowder_result with
          | Error failure ->
            Lwt.return
              ( Some ("www_hgdiff.txt", hgdiff),
                Printf.sprintf
                  "hg patch --no-commit www_hgdiff.txt\n\nnote: clowder failed to put:\n%s"
                  failure )
          | Ok clowder_script ->
            Lwt.return
              ( Some ("www_hgdiff.txt", hgdiff),
                clowder_script ^ " | hg patch --no-commit -" ))
    in
    let%lwt hg_st_result =
      Lwt_utils.exec_checked
        ?env:hgplain_env
        Exec_command.Hg
        ~timeout:30.0
        [| "status"; "--cwd"; Path.to_string env.root |]
    in
    let hg_st =
      match hg_st_result with
      | Error failure -> format_failure "Unable to `hg status`" failure
      | Ok { Lwt_utils.Process_success.stdout; _ } -> "hg status:\n" ^ stdout
    in
    Lwt.return
      ( patch_item,
        Printf.sprintf
          "hg update -C %s\n\n%s\n\n\n%s"
          mergebase
          patch_instructions
          hg_st )

let rage_www_errors (env : env) : string Lwt.t =
  let%lwt www_errors_result =
    Lwt_utils.exec_checked
      Exec_command.Hh
      ~timeout:60.0
      [|
        "check";
        "--from";
        "rage";
        "--autostart-server";
        "false";
        Path.to_string env.root;
      |]
  in
  match www_errors_result with
  | Ok success ->
    let open Lwt_utils.Process_success in
    Lwt.return
      (Printf.sprintf
         "hh check success\nCMDLINE: %s\nEXIT STATUS: 0\nSTART: %s\nEND: %s\n\nSTDOUT:\n%s\n\nSTDERR:\n%s\n"
         success.command_line
         (Utils.timestring success.start_time)
         (Utils.timestring success.end_time)
         success.stdout
         success.stderr)
  | Error failure -> Lwt.return (format_failure "hh check failure" failure)

let rage_saved_state (env : env) : (string * string) list Lwt.t =
  let watchman_opts =
    { Saved_state_loader.Watchman_options.root = env.root; sockname = None }
  in
  let saved_state_check saved_state_type =
    try%lwt
      let%lwt result_or_timeout =
        Lwt.pick
          [
            (let%lwt result =
               State_loader_lwt.load_internal
                 ~watchman_opts
                 ~ignore_hh_version:false
                 ~saved_state_type
             in
             Lwt.return_ok result);
            (let%lwt () = Lwt_unix.sleep 90.0 in
             Lwt.return_error ());
          ]
      in
      match result_or_timeout with
      | Error () -> Lwt.return_error "timeout"
      | Ok
          (Ok
            ( { Saved_state_loader.saved_state_info; changed_files; _ },
              telemetry )) ->
        Lwt.return_ok
          ( saved_state_info,
            Printf.sprintf
              "%s\n\n%s\n"
              ( List.map changed_files ~f:Relative_path.suffix
              |> String.concat ~sep:"\n" )
              (Telemetry.to_json telemetry |> Hh_json.json_to_multiline) )
      | Ok (Error (load_error, telemetry)) ->
        Lwt.return_error
          (Printf.sprintf
             "%s\n\n%s\n\n%s\n"
             (Saved_state_loader.medium_user_message_of_error load_error)
             (Saved_state_loader.debug_details_of_error load_error)
             (Telemetry.to_json telemetry |> Hh_json.json_to_multiline))
    with e -> Lwt.return_error (Exception.wrap e |> Exception.to_string)
  in
  let path_to_string path =
    let path = Path.to_string path in
    let stat = Sys_utils.lstat path in
    Printf.sprintf "%s [%d]" path stat.Unix.st_size
  in

  let%lwt naming_saved_state =
    saved_state_check Saved_state_loader.Naming_table
  in
  let naming_saved_state =
    match naming_saved_state with
    | Error s -> s
    | Ok (result, s) ->
      let open Saved_state_loader.Naming_table_info in
      Printf.sprintf
        "naming_table: %s\n\n%s"
        (path_to_string result.naming_table_path)
        s
  in

  let%lwt regular_saved_state =
    saved_state_check Saved_state_loader.Naming_and_dep_table
  in
  let regular_saved_state =
    match regular_saved_state with
    | Error s -> s
    | Ok (result, s) ->
      let open Saved_state_loader.Naming_and_dep_table_info in
      Printf.sprintf
        "naming_table: %s\ndeptable: %s\nhot_decls: %s\n\n%s"
        (path_to_string result.naming_table_path)
        (path_to_string result.dep_table_path)
        (path_to_string result.hot_decls_path)
        s
  in
  Lwt.return
    [
      ("saved_state_naming", naming_saved_state);
      ("saved_state_regular", regular_saved_state);
    ]

let rage_tmp_dir () : string Lwt.t =
  (* `ls -ld /tmp/hh_server` will show the existence, ownership and permissions of
  our tmp directory - in case hh_server hasn't been able to work right because it
  lacks ownership. *)
  let%lwt dir1_result =
    Lwt_utils.exec_checked
      Exec_command.Ls
      ~timeout:60.0
      [| "-ld"; GlobalConfig.tmp_dir |]
  in
  let dir1 =
    match dir1_result with
    | Ok { Lwt_utils.Process_success.command_line; stdout; _ } ->
      Printf.sprintf "%s\n\n%s\n\n" command_line stdout
    | Error failure -> format_failure "listing tmp directory" failure
  in
  (* `ls -lR /tmp/hh_server` will do a recursive list of every file and directory within
  our tmp directory - in case wrong files are there, or in case we lack permissions. *)
  let%lwt dir2_result =
    Lwt_utils.exec_checked
      Exec_command.Ls
      ~timeout:60.0
      [| "-lR"; GlobalConfig.tmp_dir |]
  in
  let dir2 =
    match dir2_result with
    | Ok { Lwt_utils.Process_success.command_line; stdout; _ } ->
      Printf.sprintf "%s\n\n%s\n\n" command_line stdout
    | Error failure ->
      format_failure "listing contents of tmp directory" failure
  in
  Lwt.return (dir1 ^ "\n\n" ^ dir2)

let rage_experiments_and_config
    (hhconfig_version_raw : Config_file.version option) : string list * string =
  match hhconfig_version_raw with
  | None -> ([], "")
  | Some version ->
    let config_overrides = SMap.empty in
    let local_config =
      ServerLocalConfig.load
        ~silent:true
        ~current_version:version
        config_overrides
    in
    ( local_config.ServerLocalConfig.experiments,
      local_config.ServerLocalConfig.experiments_config_meta )

let main (env : env) : Exit_status.t Lwt.t =
  let start_time = Unix.gettimeofday () in
  Hh_logger.Level.set_min_level Hh_logger.Level.Error;

  (* If user invoked us with `--rageid`, that's their way of saying that they
  want rageid to be recorded even if they terminate.
  Unix behavior when a process terminates, is that all its children get
  reparented onto ID1; also, if the process was a "session leader" then
  its children and descendents get sent SIGHUP, and their default response
  is to terminate. So we'll ignore SIGHUP in this case; also, since our
  stdout+stderr may have been closed, we'll do without them. *)
  let nohup = Option.is_some env.rageid in
  if nohup then Sys.set_signal Sys.sighup Sys.Signal_ignore;
  let printf s = (try Printf.printf "%s\n%!" s with _ when nohup -> ()) in
  let eprintf s = (try Printf.eprintf "%s\n%!" s with _ when nohup -> ()) in

  (* helpers for constructing our list of items *)
  let items : (string * string) list ref = ref [] in
  let add item = items := item :: !items in
  (* If the file exists, we'll add it. If the file doesn't exist, we won't.
  If the file exists but there was a error reading it, we'll report that error. *)
  let add_fn name fn =
    let contents = (try Sys_utils.cat fn with e -> Exn.to_string e) in
    if Sys.file_exists fn then add (name, contents)
  in

  (* strobelight *)
  eprintf "Strobelight (this takes 30s...)";
  let%lwt strobelight = rage_strobelight () in
  add ("strobelight", strobelight);

  (* stacks of processes *)
  eprintf "Fetching pstacks (this takes a minute...)";
  let%lwt pstacks = rage_pstacks env in
  add ("pstacks", pstacks);
  let%lwt ps = rage_ps () in
  add ("ps", ps);

  (* hhconfig, hh.conf *)
  let hhconfig_file = Filename.concat (Path.to_string env.root) ".hhconfig" in
  add_fn "hhconfig.txt" hhconfig_file;
  add_fn "hh_conf.txt" ServerLocalConfig.path;

  (* sandcastle *)
  begin
    match Sys.getenv_opt "SANDCASTLE_NEXUS" with
    | None -> ()
    | Some sandcastle_nexus ->
      let dir = Filename.concat sandcastle_nexus "variables" in
      let fns = (try Sys.readdir dir with _ -> [||]) in
      let fns =
        fns |> Array.to_list |> List.map ~f:(fun fn -> Filename.concat dir fn)
      in
      let fns = "/tmp/sandcastle.capabilities" :: fns in
      let contents =
        List.map fns ~f:(fun fn ->
            let content =
              try Sys_utils.cat fn |> String_utils.truncate 10240
              with e -> e |> Exception.wrap |> Exception.to_string
            in
            Printf.sprintf "%s\n%s" fn content)
      in
      add ("sandcastle", String.concat contents ~sep:"\n\n")
  end;

  (* version *)
  let%lwt hash_and_config = Config_file_lwt.parse_hhconfig hhconfig_file in
  let hhconfig_version_raw =
    match hash_and_config with
    | Error _ -> None
    | Ok (_hash, config) ->
      let version =
        SMap.find_opt "version" config |> Config_file.parse_version
      in
      Some version
  in
  let hhconfig_version =
    Option.bind hhconfig_version_raw ~f:Config_file.version_to_string_opt
  in
  let%lwt hh_version = rage_hh_version env hhconfig_version_raw in
  add ("hh_version", hh_version);

  (* hh_server internal state *)
  eprintf "Getting current hh state";
  let%lwt hh_server_state = rage_hh_server_state env in
  begin
    match hh_server_state with
    | Ok items -> List.iter items ~f:add
    | Error s -> add ("hh_server_state", s)
  end;

  (* www *)
  eprintf "Getting current www state";
  let%lwt (www_item, www_instructions) = rage_www env in
  Option.iter www_item ~f:add;
  add ("www", www_instructions);

  (* www errors *)
  eprintf "Executing hh";
  let%lwt www_errors = rage_www_errors env in
  add ("www errors", www_errors);

  (* Saved state *)
  eprintf "Checking saved-states";
  let%lwt saved_state_items = rage_saved_state env in
  List.iter saved_state_items ~f:add;

  (* Experiments *)
  let (experiments, experiments_config_meta) =
    rage_experiments_and_config hhconfig_version_raw
  in
  let experiments_content =
    Printf.sprintf
      "EXPERIMENTS\n%s\n\nEXPERIMENTS_CONFIG_META\n%s"
      (String.concat experiments ~sep:"\n")
      experiments_config_meta
  in
  add ("experiments", experiments_content);

  (* logfiles *)
  add_fn "log_server.txt" (ServerFiles.log_link env.root);
  add_fn "logold_server.txt" (ServerFiles.log_link env.root ^ ".old");
  add_fn "log_monitor.txt" (ServerFiles.monitor_log_link env.root);
  add_fn "logold_monitor.txt" (ServerFiles.monitor_log_link env.root ^ ".old");
  add_fn "log_client.txt" (ServerFiles.client_log env.root);
  add_fn "logold_client.txt" (ServerFiles.client_log env.root ^ ".old");
  add_fn "log_client_lsp.txt" (ServerFiles.client_lsp_log env.root);
  add_fn "logold_client_lsp.txt" (ServerFiles.client_lsp_log env.root ^ ".old");
  add_fn "log_client_ide.txt" (ServerFiles.client_ide_log env.root);
  add_fn "logold_client_ide.txt" (ServerFiles.client_ide_log env.root ^ ".old");

  (* temp directories *)
  eprintf "Looking at hh_server tmp directory";
  let%lwt tmp_dir = rage_tmp_dir () in
  add ("hh_server tmp", tmp_dir);

  (* We've assembled everything! now log it. *)
  let%lwt result =
    Flytrap.create ~title:("hh_rage: " ^ env.desc) ~items:!items
  in
  HackEventLogger.Rage.rage
    ~rageid:(Option.value env.rageid ~default:(Random_id.short_string ()))
    ~desc:env.desc
    ~root:env.root
    ~from:env.from
    ~hhconfig_version
    ~experiments
    ~experiments_config_meta
    ~items:!items
    ~result
    ~start_time;

  match result with
  | Ok path ->
    printf path;
    Lwt.return Exit_status.No_error
  | Error e ->
    printf ("Flytrap: failed\n" ^ e);
    Lwt.return Exit_status.Uncaught_exception

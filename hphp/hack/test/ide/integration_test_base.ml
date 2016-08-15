open Core
open Integration_test_base_types

let root = "/"
let stats = ServerMain.empty_stats ()
let genv = ref ServerEnvBuild.default_genv

let setup_server () =
  Printexc.record_backtrace true;
  EventLogger.init (Daemon.devnull ()) 0.0;
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  let _ = SharedMem.init GlobalConfig.default_sharedmem_config in
  ServerEnvBuild.make_env !genv.ServerEnv.config

let run_loop_once env inputs =
  let client_provider = ClientProvider.provider_for_test () in

  let disk_changes =
    List.map inputs.disk_changes (fun (x, y) -> root ^ x, y) in

  List.iter disk_changes
    (fun (path, contents) -> TestDisk.set path contents);

  let did_read_disk_changes_ref = ref false in

  let genv = { !genv with
    ServerEnv.notifier = begin fun () ->
      if not !did_read_disk_changes_ref then begin
        did_read_disk_changes_ref := true;
        SSet.of_list (List.map disk_changes fst)
      end else SSet.empty
    end
  } in

  let env = ServerMain.serve_one_iteration genv env client_provider stats in
  env, {
    did_read_disk_changes = !did_read_disk_changes_ref
  }

let fail x =
  print_endline x;
  exit 1

let assertEqual expected got =
  if expected <> got then fail
    (Printf.sprintf "Expected:\n%s\nGot:\n%s\n" expected got)

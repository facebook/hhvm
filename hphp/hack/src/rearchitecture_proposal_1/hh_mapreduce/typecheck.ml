(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let global_init_and_get_tcopt ~(root : Path.t) ~(hhi_root : Path.t) :
    GlobalOptions.t =
  Relative_path.set_path_prefix Relative_path.Root root;
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");

  let server_args = ServerArgs.default_options ~root:(Path.to_string root) in
  (* server_args.check_mode is used (1) by make_genv in case it wants to
     initializate watchman/dfind, (2) in serverMain and other server logic
     to affect the actual behavior. For sake of a typecheck worker here,
     we want make_genv to bypass the initialization, and we're not going to
     call into the server proper anyway. *)
  let server_args = ServerArgs.set_check_mode server_args true in
  let (server_config, server_local_config) =
    ServerConfig.load ServerConfig.filename server_args
  in
  let genv =
    ServerEnvBuild.make_genv
      server_args
      server_config
      server_local_config
      [] (* no workers *)
      None
    (* no lru_workers *)
  in
  let server_env = ServerEnvBuild.make_env genv.ServerEnv.config in
  let server_env =
    {
      server_env with
      ServerEnv.tcopt =
        {
          server_env.ServerEnv.tcopt with
          GlobalOptions.tco_shallow_class_decl = true;
        };
    }
  in
  Provider_config.set_decl_service_backend ();
  Parser_options_provider.set server_env.ServerEnv.popt;
  GlobalNamingOptions.set server_env.ServerEnv.tcopt;
  server_env.ServerEnv.tcopt

let errors_to_string (errors : Errors.t) : string =
  let one_error_to_string (e : Pos.absolute Errors.error_) : string =
    let code = Errors.get_code e in
    let msg_list = Errors.to_list e in
    let (pos, msg) = List.hd_exn msg_list in
    let (line, start, end_) = Pos.info_pos pos in
    let fn = Pos.filename pos in
    Printf.sprintf "%s(%d:%d-%d) - [%d] - %s\n" fn line start end_ code msg
  in
  let error_list =
    Errors.get_sorted_error_list errors
    |> List.map ~f:Errors.to_absolute
    |> List.map ~f:one_error_to_string
  in
  String.concat error_list

let run_worker (fd : Unix.file_descr) : unit =
  let pfd = Prototype.file_descr fd in
  (* Message1: receive root *)
  let (root, hhi_root) : Path.t * Path.t =
    match Prototype.rpc_read pfd with
    | Ok v -> v
    | Error e -> failwith (Prototype.rpc_error_to_verbose_string e)
  in
  let tcopt = global_init_and_get_tcopt ~root ~hhi_root in
  let ctx = Provider_context.empty ~tcopt in
  (* Message2: receive filename to check *)
  let fn : string =
    match Prototype.rpc_read pfd with
    | Ok v -> v
    | Error e -> failwith (Prototype.rpc_error_to_verbose_string e)
  in
  let relative_path = Relative_path.create_detect_prefix fn in
  let file_input = ServerCommandTypes.FileName fn in
  let (ctx, entry) =
    Provider_utils.update_context ctx relative_path file_input
  in
  let (errors, tast) = Provider_utils.compute_tast_and_errors ~ctx ~entry in
  let result =
    Printf.sprintf
      "ERRORS\n%s\nTAST\n%s"
      (errors_to_string errors)
      (Tast.show_program tast)
  in
  match Prototype.rpc_write pfd result with
  | Ok () -> ()
  | Error e -> failwith (Prototype.rpc_error_to_verbose_string e)

let run () : unit =
  (* Parse command-line arguments *)
  let root = ref "" in
  let files = ref [] in
  let usage =
    Printf.sprintf "Usage: %s typecheck --root [root] [files...]" Sys.argv.(0)
  in
  let options = [Args.root root] in
  Arg.parse options (fun s -> files := s :: !files) usage;
  let files = List.rev !files in
  (* TODO: handle exceptions in argument parsing, including in root *)

  (* Validate command-line arguments *)
  let files =
    match files with
    | [] ->
      Printf.eprintf "%s" usage;
      exit 1
    | "typecheck" :: files -> files
    | _ -> failwith "expected 'typecheck' as first anon argument"
  in
  (* Write out hhi files to disk *)
  (* TODO: it should be the decl-service who writes these to disk *)
  let hhi_root = Hhi.get_hhi_root () in
  (* Simplistically we'll typecheck one file at a time, one per worker *)
  let per_file (fn : string) : unit =
    let open Result.Monad_infix in
    let res =
      Prototype.rpc_request_new_worker !root Dispatch.Typecheck
      >>= fun fd ->
      Prototype.rpc_write fd (Path.make !root, hhi_root)
      >>= fun () ->
      Prototype.rpc_write fd fn
      >>= fun () ->
      Prototype.rpc_read fd
      >>= fun results ->
      Prototype.rpc_close_no_err fd;
      Ok results
    in
    match res with
    | Ok results -> Printf.printf "FILE %s\n%s\n" fn results
    | Error e ->
      Printf.eprintf
        "Prototype error: %s"
        (Prototype.rpc_error_to_verbose_string e);
      exit 1
  in
  List.iter files ~f:per_file;

  Printf.printf "Typechecking done\n";
  ()

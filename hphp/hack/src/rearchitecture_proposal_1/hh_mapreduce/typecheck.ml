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
  Parser_options_provider.set server_env.ServerEnv.popt;
  GlobalNamingOptions.set server_env.ServerEnv.tcopt;
  server_env.ServerEnv.tcopt

let run_worker (fd : Unix.file_descr) : unit =
  (* Message1: receive root *)
  let (root, hhi_root) : Path.t * Path.t =
    match Prototype.rpc_read fd with
    | Ok v -> v
    | Error e -> failwith (Prototype.rpc_error_to_verbose_string e)
  in
  let tcopt = global_init_and_get_tcopt ~root ~hhi_root in
  let ctx = Provider_context.empty ~tcopt in
  (* Message2: receive filename to check *)
  let fn : string =
    match Prototype.rpc_read fd with
    | Ok v -> v
    | Error e -> failwith (Prototype.rpc_error_to_verbose_string e)
  in
  let relative_path = Relative_path.create_detect_prefix fn in
  let file_input = ServerCommandTypes.FileName fn in
  let (ctx, entry) =
    Provider_utils.update_context ctx relative_path file_input
  in
  let tast = Provider_utils.compute_tast ~ctx ~entry in
  let result = Tast.show_program tast in
  match Prototype.rpc_write fd result with
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
        "Can't connect to prototype\n%s\n"
        (Prototype.rpc_error_to_verbose_string e);
      exit 1
  in
  List.iter files ~f:per_file;

  Printf.printf "Typechecking done\n";
  ()

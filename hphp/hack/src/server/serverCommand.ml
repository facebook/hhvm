(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open ServerCommandTypes

module TLazyHeap = Typing_lazy_heap

(* Some client commands require full check to be run in order to update global
 * state that they depend on *)
let rpc_command_needs_full_check : type a. a t -> bool = function
  (* global error list is not updated during small checks *)
  | STATUS -> true
  (* newly subscribed client will need a full list of errors *)
  | SUBSCRIBE_DIAGNOSTIC _ -> true
  (* some Ai stuff - calls to those will likely never be interleaved with IDE
   * file sync commands (and resulting small checks), but putting it here just
   * to be safe *)
  | FIND_DEPENDENT_FILES _ -> true
  | TRACE_AI _ -> true
  | AI_QUERY _ -> true
  (* Finding references uses global dependency table *)
  | FIND_REFS _ -> true
  | IDE_FIND_REFS _ -> true
  (* Codebase-wide rename, uses find references *)
  | REFACTOR _ -> true
  (* Same case as Ai commands *)
  | CREATE_CHECKPOINT _ -> true
  | RETRIEVE_CHECKPOINT _ -> true
  | DELETE_CHECKPOINT _ -> true
  | _ -> false

let command_needs_full_check = function
  | Rpc x -> rpc_command_needs_full_check x
  | Stream BUILD _ -> true (* Build doesn't fully support lazy decl *)
  | _ -> false

let full_recheck_if_needed genv env msg =
  if not env.ServerEnv.needs_full_check then env else
  if not @@ command_needs_full_check msg then env else
  let env, _, _ = ServerTypeCheck.(check genv env Full_check) in
  env

(****************************************************************************)
(* Called by the client *)
(****************************************************************************)
let rpc : type a. Timeout.in_channel * out_channel -> a t -> a
= fun (_, oc) cmd ->
  Marshal.to_channel oc (Rpc cmd) [];
  flush oc;
  let fd = Unix.descr_of_out_channel oc in
  Marshal_tools.from_fd_with_preamble fd

let rec wait_for_rpc_response fd push_messages =
  match Marshal_tools.from_fd_with_preamble fd with
  | Response r -> r, List.rev push_messages
  | Push m -> wait_for_rpc_response fd (m :: push_messages)

let rpc_persistent :
  type a. Timeout.in_channel * out_channel -> a t -> a * push list
= fun (_, oc) cmd ->
  Marshal.to_channel oc (Rpc cmd) [];
  flush oc;
  let fd = Unix.descr_of_out_channel oc in
  wait_for_rpc_response fd []

let stream_request oc cmd =
  Marshal.to_channel oc (Stream cmd) [];
  flush oc

let connect_debug oc =
  Marshal.to_channel oc Debug [];
  flush oc

let send_connection_type oc t =
  Marshal.to_channel oc t [];
  flush oc

(****************************************************************************)
(* Called by the server *)
(****************************************************************************)
let stream_response (genv:ServerEnv.genv) env (ic, oc) ~cmd =
  match cmd with
  | LIST_FILES ->
      ServerEnv.list_files env oc;
      ServerUtils.shutdown_client (ic, oc)
  | LIST_MODES ->
      Relative_path.Map.iter env.ServerEnv.files_info begin fun fn fileinfo ->
        match Relative_path.prefix fn with
        | Relative_path.Root ->
          let mode = match fileinfo.FileInfo.file_mode with
            | None -> "php"
            | Some FileInfo.Mdecl -> "decl"
            | Some FileInfo.Mpartial -> "partial"
            | Some FileInfo.Mstrict -> "strict" in
          Printf.fprintf oc "%s\t%s\n" mode (Relative_path.to_absolute fn)
        | _ -> ()
      end;
      flush oc;
      ServerUtils.shutdown_client (ic, oc)
  | SHOW name ->
      output_string oc "starting\n";
      SharedMem.invalidate_caches();
      let qual_name = if name.[0] = '\\' then name else ("\\"^name) in
      output_string oc "class:\n";
      let class_name =
        match NamingGlobal.GEnv.type_canon_name qual_name with
        | None ->
          let () = output_string oc "Missing from naming env\n" in qual_name
        | Some canon ->
          let p = unsafe_opt @@ NamingGlobal.GEnv.type_pos canon in
          let () = output_string oc ((Pos.string (Pos.to_absolute p))^"\n") in
          canon
      in
      let class_ = TLazyHeap.get_class env.ServerEnv.tcopt class_name in
      (match class_ with
      | None -> output_string oc "Missing from typing env\n"
      | Some c ->
          let class_str = Typing_print.class_ env.ServerEnv.tcopt c in
          output_string oc (class_str^"\n")
      );
      output_string oc "\nfunction:\n";
      let fun_name =
        match NamingGlobal.GEnv.fun_canon_name qual_name with
        | None ->
          let () = output_string oc "Missing from naming env\n" in qual_name
        | Some canon ->
          let p = unsafe_opt @@ NamingGlobal.GEnv.fun_pos canon in
          let () = output_string oc ((Pos.string (Pos.to_absolute p))^"\n") in
          canon
      in
      let fun_ = TLazyHeap.get_fun env.ServerEnv.tcopt fun_name in
      (match fun_ with
      | None ->
          output_string oc "Missing from typing env\n"
      | Some f ->
          let fun_str = Typing_print.fun_ f in
          output_string oc (fun_str^"\n")
      );
      output_string oc "\nglobal const:\n";
      (match NamingGlobal.GEnv.gconst_pos qual_name with
      | Some p -> output_string oc (Pos.string (Pos.to_absolute p)^"\n")
      | None -> output_string oc "Missing from naming env\n");
      let gconst_ty = TLazyHeap.get_gconst env.ServerEnv.tcopt qual_name in
      (match gconst_ty with
      | None -> output_string oc "Missing from typing env\n"
      | Some gc ->
          let gconst_str = Typing_print.gconst gc in
          output_string oc ("ty: "^gconst_str^"\n")
      );
      output_string oc "typedef:\n";
      (match NamingGlobal.GEnv.typedef_pos qual_name with
      | Some p -> output_string oc (Pos.string (Pos.to_absolute p)^"\n")
      | None -> output_string oc "Missing from naming env\n");
      let tdef = TLazyHeap.get_typedef env.ServerEnv.tcopt qual_name in
      (match tdef with
      | None ->
          output_string oc "Missing from typing env\n"
      | Some td ->
          let td_str = Typing_print.typedef td in
          output_string oc (td_str^"\n")
      );
      flush oc;
      ServerUtils.shutdown_client (ic, oc)
  | BUILD build_opts ->
      let build_hook = BuildMain.go build_opts genv env oc in
      (match build_hook with
      | None -> ServerUtils.shutdown_client (ic, oc)
      | Some build_hook -> begin
        ServerTypeCheck.hook_after_parsing :=
          Some (fun genv env ->
            (* subtle: an exception there (such as writing on a closed pipe)
             * will not be caught by handle_connection() because
             * we have already returned from handle_connection(), hence
             * this additional try.
             *)
            (try
              with_context
                ~enter:(fun () -> ())
                ~exit:(fun () -> ServerUtils.shutdown_client (ic, oc))
                ~do_:(fun () -> build_hook genv env);
            with exn ->
              let msg = Printexc.to_string exn in
              Printf.printf "Exn in build_hook: %s" msg;
              EventLogger.master_exception msg;
            );
            ServerTypeCheck.hook_after_parsing := None
          )
      end)

let handle genv env client =
  let msg = ClientProvider.read_client_msg client in
  let env = full_recheck_if_needed genv env msg in
  match msg with
  | Rpc cmd ->
      let t = Unix.gettimeofday () in
      let new_env, response = ServerRpc.handle
        ~is_stale:env.ServerEnv.recent_recheck_loop_stats.ServerEnv.updates_stale
        genv env cmd in
      let cmd_string = ServerRpc.to_string cmd in
      HackEventLogger.handled_command cmd_string t;
      ClientProvider.send_response_to_client client response;
      if cmd = ServerCommandTypes.DISCONNECT ||
          not @@ (ClientProvider.is_persistent client)
        then ClientProvider.shutdown_client client;
      if cmd = ServerCommandTypes.KILL then ServerUtils.die_nicely ();
      new_env
  | Stream cmd ->
      let ic, oc = ClientProvider.get_channels client in
      stream_response genv env (ic, oc) ~cmd;
      env
  | Debug ->
      let ic, oc = ClientProvider.get_channels client in
      genv.ServerEnv.debug_channels <- Some (ic, oc);
      ServerDebug.say_hello genv;
      env

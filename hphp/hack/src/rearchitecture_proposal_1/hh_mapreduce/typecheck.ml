(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let run_worker (fd : Unix.file_descr) : unit =
  let req = (Marshal_tools.from_fd_with_preamble fd : string) in
  let _ = Marshal_tools.to_fd_with_preamble fd req in
  ()

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

  (* Simplistically we'll typecheck one file at a time, one per worker *)
  let per_file (fn : string) : unit =
    let open Result.Monad_infix in
    let res =
      Prototype.rpc_request_new_worker !root Dispatch.Typecheck
      >>= fun fd ->
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

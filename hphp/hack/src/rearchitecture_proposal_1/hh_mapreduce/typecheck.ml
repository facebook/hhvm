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
  let usage = Printf.sprintf "Usage: %s typecheck --root ..." Sys.argv.(0) in
  let options = [Args.root root] in
  Arg.parse options (Args.only "typecheck") usage;

  (* TODO: handle exceptions in argument parsing *)

  (* fetch two typechecker workers *)
  let resA = Prototype.rpc_request_new_worker !root Dispatch.Typecheck in
  let resB = Prototype.rpc_request_new_worker !root Dispatch.Typecheck in
  let (fdA, fdB) =
    match (resA, resB) with
    | (Ok fdA, Ok fdB) -> (fdA, fdB)
    | _ ->
      Printf.eprintf "Can't connect to prototype\n";
      exit 1
  in
  (* orchestrate the typechecker work *)
  Printf.printf "T->Wa sending 'A'\n%!";
  Printf.printf "T->Wb sending 'B'\n%!";
  let _res1A = Prototype.rpc_write fdA "A" in
  let _res1B = Prototype.rpc_write fdB "B" in
  let res2A = Prototype.rpc_read fdA in
  let res2B = Prototype.rpc_read fdB in
  begin
    match (res2A, res2B) with
    | (Ok respA, Ok respB) ->
      Printf.printf "T<-Wa response '%s'\n%!" respA;
      Printf.printf "T<-Wb response '%s'\n%!" respB
    | _ -> Printf.printf "T<-Wa/Wb failed.\n%!"
  end;
  Prototype.rpc_close_no_err fdA;
  Prototype.rpc_close_no_err fdB;

  Printf.printf "T. Done\n%!";
  ()

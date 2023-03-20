(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let solve_persisted_usage =
  "solve the constraints at `Sdt_analysis.default_db_dir` and log them to stdout"

let dump_persisted_usage =
  "dump the constraints at `Sdt_analysis.default_db_dir` to stdout"

let usage =
  Format.sprintf
    {|
"USAGE":
  --solve-persisted %s
  --dump-persisted %s
|}
    solve_persisted_usage
    dump_persisted_usage

let exit_with_usage () =
  Format.eprintf "incorrect command line arguments\n%s" usage;
  Exit.exit Exit_status.Input_error

type mode =
  | SolvePersisted
  | DumpPersisted

let () =
  let mode = ref None in

  let set_mode m =
    match !mode with
    | None -> mode := Some m
    | Some _ -> exit_with_usage ()
  in

  let args =
    [
      ( "--solve-persisted",
        Arg.Unit (fun () -> set_mode SolvePersisted),
        solve_persisted_usage );
      ( "--dump-persisted",
        Arg.Unit (fun () -> set_mode DumpPersisted),
        dump_persisted_usage );
    ]
  in
  Arg.parse
    args
    (fun s ->
      Printf.printf "Unrecognized argument: '%s'\n" s;
      Exit.exit Exit_status.Input_error)
    usage;

  match !mode with
  | None -> exit_with_usage ()
  | Some mode ->
    (match mode with
    | SolvePersisted ->
      Sdt_analysis.StandaloneApi.solve_persisted
        ~db_dir:Sdt_analysis.default_db_dir
    | DumpPersisted ->
      Sdt_analysis.StandaloneApi.dump_persisted
        ~db_dir:Sdt_analysis.default_db_dir)

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let usage =
  {|
Codemod the entire repo to add missing `__NeedsConcrete` attributes (including generated files) in order to get stats.
Best invoked with ./run.sh
Currently useful for "what-if" analysis (getting numbers).
|}

type args = {
  root: string;
  errors_file: string;
}

(** Helps keep track of things when forking processes and dividing up work *)
module Multi = struct
  type state =
    | Parent_with_child_pids of int list
    | Child

  type t = {
    work_index: int;
    pid_index: int;
    state: state;
  }

  let empty =
    { work_index = 0; pid_index = 0; state = Parent_with_child_pids [] }
end

let parse_args_exn () : args =
  let errors_file = ref None in
  let root = ref None in
  let () =
    Arg.parse
      [
        ( "--errors",
          Arg.String (fun s -> errors_file := Some s),
          "File containing errors to codemod, produce by hh --re --json" );
        ("--root", Arg.String (fun s -> root := Some s), "Where's your WWW?");
      ]
      (fun _ -> ())
      usage
  in
  let errors_file = Option.value_exn ~message:usage !errors_file in
  let root = Option.value_exn ~message:usage !root in
  let () = Printf.printf "Codemodding %s from %s\n" root errors_file in
  { root; errors_file }

let () =
  let { root; errors_file } = parse_args_exn () in
  let () = Relative_path.set_path_prefix Relative_path.Root (Path.make root) in
  let json_string = Disk.cat errors_file in
  let () = Printf.printf "Parsing to json %!\n" in
  let json = Yojson.Safe.from_string json_string in
  let error_message =
    Printf.sprintf "unexpected json found in %s:\n%s\n" errors_file json_string
  in
  let () = Printf.printf "Parsing to error information %!\n" in
  let warnings =
    Codemod_sa_warning.parse_warnings_json_exn json ~error_message
  in
  (* According to a comment in ServerMain.ml, the hardware we are running on has only 1/2
     actual CPUs, the rest are hyperthreads. We use `3` as the denominator because
     if we only divide by 2 then the devvm gets super jammed up and hard to use iirc
  *)
  let proc_count = max 0 (Sys_utils.nbr_procs / 3) in
  let work_count = Relative_path.Map.cardinal warnings in
  let bucket_size =
    (work_count / proc_count) + 1 (* TODO: make fairly distributed *)
  in
  let on_path path warnings_for_path =
    let file = Relative_path.to_absolute path in
    let code = Disk.cat file in
    let contents = Codemod_sa_rewrite.rewrite path warnings_for_path code in
    Disk.write_file ~file ~contents
  in

  let acc =
    Relative_path.Map.fold
      ~init:Multi.empty
      warnings
      ~f:(fun path warnings_for_path { work_index; pid_index; state } ->
        let start = pid_index * bucket_size in
        let end_ = min work_count ((pid_index + 1) * bucket_size) in
        let should_handle =
          pid_index >= 0 && work_index >= start && work_index < end_
        in
        let continue_as_child () : Multi.t =
          let () = on_path path warnings_for_path in
          { work_index = work_index + 1; pid_index; state = Child }
        in

        if should_handle then
          match state with
          | Parent_with_child_pids pids ->
            let () =
              Printf.printf
                "Parent delegating work %d / %d to pid_index %d%!\n"
                work_index
                work_count
                pid_index
            in
            (* Flush so child processes' don't print anything left over from the parent's buffers *)
            let () =
              Out_channel.flush stdout;
              Out_channel.flush stderr
            in
            begin
              match Unix.fork () with
              | -1 -> failwith "fork failed"
              | 0 ->
                (* in child process *)
                continue_as_child ()
              | pid ->
                (* in parent process *)
                {
                  work_index = work_index + 1;
                  pid_index = pid_index + 1;
                  state = Parent_with_child_pids (pid :: pids);
                }
            end
          | Child -> continue_as_child ()
        else
          { work_index = work_index + 1; pid_index; state })
  in
  match acc.state with
  | Parent_with_child_pids pids ->
    List.iter pids ~f:(fun pid ->
        match snd @@ Unix.waitpid [] pid with
        | WEXITED 0 -> ()
        | WEXITED n -> Printf.eprintf "pid %d WEXITED %d" pid n
        | WSIGNALED n -> Printf.eprintf "pid %d WSTOPPED %d" pid n
        | WSTOPPED n -> Printf.eprintf "pid %d WSTOPPED %d" pid n)
  | Child -> ()

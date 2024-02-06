(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This long-lived Lwt routine will keep polling the file, and the
report it finds into the queue. If it gets an error then it sticks that
in the queue and terminates.
Exception: if it gets a NothingYet error, then it either continues polling
the file, or ends the queue with Server_progress.Killed, depending on whether
the producing PID is still alive. *)
let rec watch
    ~(pid : int)
    ~(pid_future : unit Lwt.t)
    (fd : Unix.file_descr)
    (add : Server_progress.ErrorsRead.read_result option -> unit) : unit Lwt.t =
  match Server_progress.ErrorsRead.read_next_errors fd with
  | Ok errors ->
    add (Some (Ok errors));
    watch ~pid ~pid_future fd add
  | Error (Server_progress.NothingYet, _) when Lwt.is_sleeping pid_future ->
    let%lwt () = Lwt_unix.sleep 0.2 in
    watch ~pid ~pid_future fd add
  | Error (Server_progress.NothingYet, _) ->
    let server_finale_file = ServerFiles.server_finale_file pid in
    let finale_data = Exit_status.get_finale_data server_finale_file in
    add (Some (Error (Server_progress.Killed finale_data, "pid")));
    add None;
    Lwt.return_unit
  | Error e ->
    add (Some (Error e));
    add None;
    Lwt.return_unit

(** This returns an Lwt future which will complete once <pid> dies.
It implements this by polling "SIGKILL 0" every 5s. *)
let rec watch_pid (pid : int) : unit Lwt.t =
  let%lwt () = Lwt_unix.sleep 5.0 in
  let is_alive =
    try
      Unix.kill pid 0;
      true
    with
    | _ -> false
  in
  if is_alive then
    watch_pid pid
  else
    Lwt.return_unit

let watch_errors_file ~(pid : int) (fd : Unix.file_descr) :
    Server_progress.ErrorsRead.read_result Lwt_stream.t =
  let (q, add) = Lwt_stream.create () in
  let pid_future = watch_pid pid in
  let _watcher_future =
    watch ~pid ~pid_future fd add |> Lwt.map (fun () -> Lwt.cancel pid_future)
  in
  q

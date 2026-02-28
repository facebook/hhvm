(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Periodically called by the daemon *)
(*****************************************************************************)

type callback =
  | Periodic of (float ref * float * (env:ServerEnv.env -> ServerEnv.env))
  | Once of (float ref * (env:ServerEnv.env -> ServerEnv.env))

module Periodical : sig
  val always : float

  val one_second : float

  val one_minute : float

  val one_hour : float

  val one_day : float

  val one_week : float

  (** Check if any callback is due and run those. *)
  val check : ServerEnv.env -> ServerEnv.env

  (* register_callback X Y
   * Registers a new callback Y called every X seconds.
   * The time is an approximation, don't expect it to be supper accurate.
   * More or less 1 sec is a more or less what you can expect.
   * More or less 30 secs if the server is busy.
   *)
  val register_callback : callback -> unit
end = struct
  let always = 0.0

  let one_second = 1.0

  let one_minute = 60.0

  let one_hour = 3600.0

  let one_day = 86400.0

  let one_week = 604800.0

  let callback_list = ref []

  let last_call = ref (Unix.time ())

  let check (env : ServerEnv.env) : ServerEnv.env =
    let current = Unix.time () in
    let delta = current -. !last_call in
    let env = ref env in
    last_call := current;
    callback_list :=
      List.filter !callback_list ~f:(fun callback ->
          (match callback with
          | Periodic (seconds_left, _, job)
          | Once (seconds_left, job) ->
            seconds_left := !seconds_left -. delta;
            if Float.(!seconds_left < 0.0) then env := job ~env:!env);
          match callback with
          | Periodic (seconds_left, period, _) ->
            if Float.(!seconds_left < 0.0) then seconds_left := period;
            true
          | Once _ -> false);
    !env

  let register_callback cb = callback_list := cb :: !callback_list
end

let go = Periodical.check

let async f = Periodical.register_callback (Once (ref 0.0, f))

(*****************************************************************************)
(*
 * kill the server every 24h. We do this to save resources and
 * make sure everyone is +/- running the same version.
 *
 * TODO: improve this check so the server only restarts
 *       if there hasn't been any activity for x hours/days.
 *)
(*****************************************************************************)

(* We want to keep track of when the server was last used. Every few hours, we'll
 * check this variable. If the server hasn't been used for a few days, we exit.
 *)
let last_client_connect : float ref = ref (Unix.time ())

let stamp_connection () =
  last_client_connect := Unix.time ();
  ()

let exit_if_unused () =
  let delta : float = Unix.time () -. !last_client_connect in
  if Float.(delta > Periodical.one_week) then (
    Printf.eprintf "Exiting server. Last used >7 days ago\n";
    Exit.exit Exit_status.Unused_server
  )

(*****************************************************************************)
(* The registered jobs *)
(*****************************************************************************)
let init (genv : ServerEnv.genv) (root : Path.t) : unit =
  ignore genv;
  let jobs =
    [
      (* I'm not sure explicitly invoking the Gc here is necessary, but
       * major_slice takes something like ~0.0001s to run, so why not *)
      ( Periodical.always,
        fun ~env ->
          let _result : int = Gc.major_slice 0 in
          env );
      ( Periodical.one_second,
        fun ~env ->
          EventLogger.recheck_disk_files ();
          env );
      ( Periodical.one_minute *. 5.,
        fun ~env ->
          begin
            try
              (* We'll cycle the client-log if it gets bigger than 1Mb.
                 We do this cycling here in the server (rather than in the client)
                 to avoid races when multiple concurrent clients try to cycle it. *)
              let client_log_fn = ServerFiles.client_log root in
              let stat = Unix.stat client_log_fn in
              if stat.Unix.st_size > 1024 * 1024 then
                Sys.rename client_log_fn (client_log_fn ^ ".old")
            with
            | _ -> ()
          end;
          env );
      ( Periodical.one_hour *. 3.,
        fun ~env ->
          EventLogger.log_gc_stats ();
          env );
      ( Periodical.always,
        fun ~env ->
          SharedMem.GC.collect `aggressive;
          env );
      ( Periodical.always,
        fun ~env ->
          EventLogger.flush ();
          env );
      ( Periodical.one_day,
        fun ~env ->
          exit_if_unused ();
          env );
      ( Periodical.one_day,
        fun ~env ->
          Hhi.touch ();
          env );
      (* Touch_existing{false} wraps Unix.lutimes, which doesn't open/close any fds, so we
       * won't lose our lock by doing this. We are only touching the top level
       * of files, however -- we don't want to do it recursively so that old
       * files under e.g. /tmp/hh_server/logs still get cleaned up. *)
      ( Periodical.one_day,
        fun ~env ->
          Array.iter
            ~f:
              begin
                fun fn ->
                  let fn = Filename.concat GlobalConfig.tmp_dir fn in
                  if
                    (try Sys.is_directory fn with
                    | _ -> false)
                    (* We don't want to touch things like .watchman_failed *)
                    || String.is_prefix fn ~prefix:"."
                    || not (ServerFiles.is_of_root root fn)
                  then
                    ()
                  else
                    Sys_utils.try_touch
                      (Sys_utils.Touch_existing { follow_symlinks = false })
                      fn
              end
            (Sys.readdir GlobalConfig.tmp_dir);
          env );
    ]
  in
  List.iter jobs ~f:(fun (period, cb) ->
      Periodical.register_callback (Periodic (ref period, period, cb)))

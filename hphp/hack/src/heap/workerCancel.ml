(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Please read the documentation in the .mli file. *)
exception Worker_should_exit

let () = Callback.register_exception "worker_should_exit" Worker_should_exit

external stop_workers : unit -> unit = "hh_stop_workers"

external resume_workers : unit -> unit = "hh_resume_workers"

external set_can_worker_stop : bool -> unit = "hh_set_can_worker_stop"

external is_stop_requested : unit -> bool = "hh_should_exit" [@@noalloc]

external raise_if_stop_requested : unit -> unit = "hh_raise_if_should_exit"

let with_no_cancellations_nested = ref false

let with_no_cancellations f =
  if !with_no_cancellations_nested then
    failwith "with_no_cancellations cannot be nested";
  with_no_cancellations_nested := true;
  Utils.try_finally
    ~f:
      begin
        fun () ->
          set_can_worker_stop false;
          f ()
      end
    ~finally:(fun () ->
      set_can_worker_stop true;
      with_no_cancellations_nested := false)

(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * We shell out hg commands, so its computation is out-of-process, and
 * return a Future for the result.
 *
 * A promise is a strongly-typed wrapper around Process
 *)

open Hh_prelude
include Future_sig.Types

type 'a delayed = {
  (* Number of times it has been tapped by "is_ready" or "check_status". *)
  tapped: int;
  (* Number of times remaining to be tapped before it is ready. *)
  remaining: int;
  value: 'a;
}

type 'a incomplete = {
  (* The underlying implementation whose progress we manage *)
  process: Process_types.t;
  (* An optional deadline (time value); if set, the future can expire when being
    checked for readiness. If not set, only the `get` function's timeout will
    apply (default or explicit). *)
  deadline: float option;
  (* The underlying implementation's output is a string. This function transforms
    the output into a desired value of the type that the Future's successful
    result is of. *)
  transformer: string -> 'a;
}

type 'a promise =
  | Complete : 'a -> 'a promise
  | Complete_but_failed of error
  (* Delayed is useful for deterministic testing. Must be tapped by "is ready" or
   * "check_status" the remaining number of times before it is ready. *)
  | Delayed : 'a delayed -> 'a promise
  (* A future formed from two underlying futures. Calling "get" blocks until
   * both underlying are ready. *)
  | Merged :
      ( 'a t
      * 'b t
      * (('a, error) result -> ('b, error) result -> ('c, error) result) )
      -> 'c promise
  (* The future's success is bound to another future producer; it's a chain
      of futures that have to execute serially in order, as opposed to
      the list of Merged futures above that have to execute in parallel. *)
  | Bound : ('a t * (('a, error) result -> 'b t)) -> 'b promise
  (* The future is not yet fulfilled. *)
  | Incomplete of 'a incomplete

(** float is the time the Future was constructed. *)
and 'a t = 'a promise ref * float

let equal (_f : 'a -> 'a -> bool) (x : 'a t) (y : 'a t) = Poly.equal x y

(* 30 seconds *)
let default_timeout = 30

(* Given an optional deadline, constructs a timeout time span, in seconds,
    relative to the current time (dealine - now). If the current time is past
    the deadline, the timeout is 0. If the deadline is not specified,
    the max timeout value is returned. *)
let timeout_of_deadline deadline ~max_timeout =
  match deadline with
  | Some deadline ->
    let time_left = deadline -. Unix.gettimeofday () in
    if time_left > 0.0 then
      min (int_of_float time_left) max_timeout
    else
      0
  | None -> max_timeout

(* Given an optional timeout, constructs the deadline relative to
    the specified start time. If the timeout is not specified, then
    the deadline is also not specified *)
let deadline_of_timeout ~timeout ~start_time =
  match timeout with
  | Some timeout -> Some (start_time +. float_of_int timeout)
  | None -> None

let make
    (process : Process_types.t)
    ?(timeout : int option)
    (transformer : string -> 'result) : 'result t =
  let deadline =
    deadline_of_timeout ~timeout ~start_time:(Unix.gettimeofday ())
  in
  (ref (Incomplete { process; deadline; transformer }), Unix.gettimeofday ())

let of_value v = (ref @@ Complete v, Unix.gettimeofday ())

let of_error (e : string) =
  try failwith e
  with e ->
    let e = Exception.wrap e in
    let info = Process_types.dummy.Process_types.info in
    ( ref @@ Complete_but_failed (info, Continuation_raised e),
      Unix.gettimeofday () )

let delayed_value ~delays v =
  ( ref (Delayed { tapped = 0; remaining = delays; value = v }),
    Unix.gettimeofday () )

let error_to_string (info, e) =
  let info =
    Printf.sprintf
      "(%s [%s])"
      info.Process_types.name
      (String.concat ~sep:", " info.Process_types.args)
  in
  let status_string s =
    match s with
    | Unix.WEXITED i -> Printf.sprintf "(%s WEXITED %d)" info i
    | Unix.WSIGNALED i -> Printf.sprintf "(%s WSIGNALED %d)" info i
    | Unix.WSTOPPED i -> Printf.sprintf "(%s WSTOPPED %d)" info i
  in
  match e with
  | Process_failure { status; stderr } ->
    Printf.sprintf
      "Process_failure(%s, stderr: %s)"
      (status_string status)
      stderr
  | Timed_out { stdout; stderr } ->
    Printf.sprintf "Timed_out(%s (stdout: %s) (stderr: %s))" info stdout stderr
  | Process_aborted -> Printf.sprintf "Process_aborted(%s)" info
  | Continuation_raised e ->
    Printf.sprintf "Continuation_raised(%s)" (Exception.get_ctor_string e)
  | Transformer_raised e ->
    Printf.sprintf
      "Transformer_raised(%s %s)"
      info
      (Exception.get_ctor_string e)

let error_to_string_verbose (error : error) : verbose_error =
  let (invocation_info, error_mode) = error in
  let Process_types.{ name; args; env; stack = Utils.Callstack stack } =
    invocation_info
  in
  let env = Process.env_to_string env in
  let stack = stack |> Exception.clean_stack in
  let cmd_and_args =
    Printf.sprintf "`%s %s`" name (String.concat ~sep:" " args)
  in
  match error_mode with
  | Process_failure { status; stderr } ->
    let status =
      match status with
      | Unix.WEXITED i -> Printf.sprintf "exited with code %n" i
      | Unix.WSIGNALED i -> Printf.sprintf "killed with signal %n" i
      | Unix.WSTOPPED i -> Printf.sprintf "stopped with signal %n" i
    in
    {
      message = Printf.sprintf "%s - %s\n%s\n" cmd_and_args status stderr;
      environment = Some env;
      stack = Utils.Callstack stack;
    }
  | Timed_out { stdout; stderr } ->
    {
      message =
        Printf.sprintf
          "%s timed out\nSTDOUT:\n%s\nSTDERR:\n%s\n"
          cmd_and_args
          stdout
          stderr;
      environment = Some env;
      stack = Utils.Callstack stack;
    }
  | Process_aborted ->
    {
      message = Printf.sprintf "%s aborted" cmd_and_args;
      environment = None;
      stack = Utils.Callstack stack;
    }
  | Continuation_raised e ->
    let stack =
      (Exception.get_backtrace_string e |> Exception.clean_stack)
      ^ "-----\n"
      ^ stack
    in
    {
      message =
        Printf.sprintf "Continuation failure - %s" (Exception.get_ctor_string e);
      environment = None;
      stack = Utils.Callstack stack;
    }
  | Transformer_raised e ->
    let stack =
      (Exception.get_backtrace_string e |> Exception.clean_stack)
      ^ "-----\n"
      ^ stack
    in
    {
      message =
        Printf.sprintf
          "%s - unable to process output - %s"
          cmd_and_args
          (Exception.get_ctor_string e);
      environment = None;
      stack = Utils.Callstack stack;
    }

let error_to_exn e = Failure e

let rec get : 'a. ?timeout:int -> 'a t -> ('a, error) result =
 fun ?(timeout = default_timeout) (promise, _) ->
  match !promise with
  | Complete v -> Ok v
  | Complete_but_failed e -> Error e
  | Delayed { value; remaining; _ } when remaining <= 0 -> Ok value
  | Delayed _ ->
    let error =
      Timed_out { stdout = ""; stderr = "Delayed value not ready yet" }
    in
    Error (Process_types.dummy.Process_types.info, error)
  | Merged (a, b, handler) ->
    let start_t = Unix.time () in
    let a = get ~timeout a in
    let consumed_t = int_of_float @@ (Unix.time () -. start_t) in
    let timeout = timeout - consumed_t in
    let b = get ~timeout b in
    (* NB: We don't need to cache the result of running the handler because
     * underlying Futures a and b have the values cached internally. So
     * subsequent calls to "get" on this Merged Future will just re-run
     * the handler on the cached result. *)
    handler a b
  | Bound (curr_future, next_producer) ->
    let start_t = Unix.time () in
    let curr_result = get ~timeout curr_future in
    let consumed_t = int_of_float @@ (Unix.time () -. start_t) in
    let timeout = timeout - consumed_t in
    begin
      try
        let next_future = next_producer curr_result in
        let next_result = get ~timeout next_future in
        (* The `get` call above changes next_promise's internal state/cache, so
          the updating of the promise below should happen AFTER calling `get`
          on it. *)
        let (next_promise, _t) = next_future in
        promise := !next_promise;
        next_result
      with e ->
        let e = Exception.wrap e in
        let info = Process_types.dummy.Process_types.info in
        promise := Complete_but_failed (info, Continuation_raised e);
        Error (info, Continuation_raised e)
    end
  | Incomplete { process; deadline; transformer } ->
    let timeout = timeout_of_deadline deadline ~max_timeout:timeout in
    let info = process.Process_types.info in
    (match Process.read_and_wait_pid ~timeout process with
    | Ok { Process_types.stdout; _ } ->
      begin
        try
          let result = transformer stdout in
          promise := Complete result;
          Ok result
        with e ->
          let e = Exception.wrap e in
          promise := Complete_but_failed (info, Transformer_raised e);
          Error (info, Transformer_raised e)
      end
    | Error (Process_types.Abnormal_exit { status; stderr; _ }) ->
      Error (info, Process_failure { status; stderr })
    | Error (Process_types.Timed_out { stdout; stderr }) ->
      Error (info, Timed_out { stdout; stderr })
    | Error Process_types.Overflow_stdin -> Error (info, Process_aborted))

let get_exn ?timeout x =
  get ?timeout x |> Result.map_error ~f:error_to_exn |> Result.ok_exn

(* Must explicitly make recursive functions polymorphic. *)
let rec is_ready : 'a. 'a t -> bool =
 fun (promise, _) ->
  match !promise with
  | Complete _
  | Complete_but_failed _ ->
    true
  | Delayed { remaining; _ } when remaining <= 0 -> true
  | Delayed { tapped; remaining; value } ->
    promise := Delayed { tapped = tapped + 1; remaining = remaining - 1; value };
    false
  | Merged (a, b, _) ->
    (* Prevent the && operator from short-cirtuiting the is-ready check for
        the second future: *)
    let is_a_ready = is_ready a in
    let is_b_ready = is_ready b in
    is_a_ready && is_b_ready
  | Bound (curr_future, next_producer) ->
    if is_ready curr_future then begin
      let curr_result = get curr_future in
      try
        let next_future = next_producer curr_result in
        let is_next_ready = is_ready next_future in
        (* `is_ready` *may* change next_promise's internal state/cache, so
        the updating of the promise below should happen AFTER calling `is_ready`
        on it. *)
        let (next_promise, _t) = next_future in
        promise := !next_promise;
        is_next_ready
      with e ->
        let e = Exception.wrap e in
        let info = Process_types.dummy.Process_types.info in
        promise := Complete_but_failed (info, Continuation_raised e);
        true
    end else
      false
  | Incomplete { process; deadline; _ } ->
    (* Note: if the promise's own deadline is not set, we allow the caller
        to call is_ready as long as they wish, without timing out *)
    let timeout = timeout_of_deadline deadline ~max_timeout:1 in
    if timeout > 0 then
      Process.is_ready process
    else
      (* E.g., we timed out *)
      true

let merge_status stat_a stat_b handler =
  match (stat_a, stat_b) with
  | (Complete_with_result a, Complete_with_result b) ->
    Complete_with_result (handler a b)
  | (In_progress { age }, In_progress { age = age_b }) when Float.(age > age_b)
    ->
    In_progress { age }
  | (In_progress { age }, _)
  | (_, In_progress { age }) ->
    In_progress { age }

let start_t : 'a. 'a t -> float = (fun (_, time) -> time)

let merge a b handler =
  (ref (Merged (a, b, handler)), Float.min (start_t a) (start_t b))

let make_continue (first : 'first t) next_producer : 'next t =
  (ref (Bound (first, next_producer)), start_t first)

let continue_with_future (a : 'a t) (f : 'a -> 'b t) : 'b t =
  let f res =
    match res with
    | Ok res -> f res
    | Error error -> (ref (Complete_but_failed error), start_t a)
  in
  make_continue a f

let continue_with (a : 'a t) (f : 'a -> 'b) : 'b t =
  continue_with_future a (fun a -> of_value (f a))

let continue_and_map_err (a : 'a t) (f : ('a, error) result -> ('b, 'c) result)
    : ('b, 'c) result t =
  let f res = of_value (f res) in
  make_continue a f

let on_error (future : 'value t) (f : error -> unit) : 'value t =
  let continuation res =
    match res with
    | Ok res -> of_value res
    | Error error ->
      f error;
      (ref (Complete_but_failed error), start_t future)
  in
  make_continue future continuation

(* Must explicitly make recursive functions polymorphic. *)
let rec with_timeout : 'value. 'value t -> timeout:int -> 'value t =
 fun ((promise, start_time) as future) ~timeout ->
  match !promise with
  | Complete _
  | Complete_but_failed _ ->
    future
  | Delayed _ ->
    (* The delayed state is used for testing, so it doesn't make sense to
        figure out the semantics of what it means to have a timeout on a
        delayed future *)
    failwith "Setting timeout on a delayed future is not supported"
  | Merged (a_future, b_future, handler) ->
    merge
      (with_timeout a_future ~timeout)
      (with_timeout b_future ~timeout)
      handler
  | Bound (curr_future, next_producer) ->
    make_continue (with_timeout curr_future ~timeout) next_producer
  | Incomplete { process; transformer; _ } ->
    let deadline = deadline_of_timeout ~timeout:(Some timeout) ~start_time in
    (ref (Incomplete { process; deadline; transformer }), start_time)

(* Must explicitly make recursive function polymorphic. *)
let rec check_status : 'a. 'a t -> 'a status =
 fun (promise, start_t) ->
  match !promise with
  | Complete v -> Complete_with_result (Ok v)
  | Complete_but_failed e -> Complete_with_result (Error e)
  | Delayed { value; remaining; _ } when remaining <= 0 ->
    Complete_with_result (Ok value)
  | Delayed { tapped; remaining; value } ->
    promise := Delayed { tapped = tapped + 1; remaining = remaining - 1; value };
    In_progress { age = float_of_int tapped }
  | Merged (a, b, handler) ->
    merge_status (check_status a) (check_status b) handler
  | Bound _ ->
    if is_ready (promise, start_t) then
      Complete_with_result (get (promise, start_t))
    else
      let age = Unix.time () -. start_t in
      In_progress { age }
  | Incomplete { process; deadline; _ } ->
    (* Note: if the promise's own deadline is not set, we allow the caller
        to call check_status as long as they wish, without timing out *)
    let timeout = timeout_of_deadline deadline ~max_timeout:1 in
    if Process.is_ready process || timeout <= 0 then
      Complete_with_result (get ~timeout (promise, start_t))
    else
      let age = Unix.time () -. start_t in
      In_progress { age }

(* Necessary to avoid a cyclic type definition error. *)
type 'a future = 'a t

module Promise = struct
  type 'a t = 'a future

  let return = of_value

  let map = continue_with

  let bind = continue_with_future
end

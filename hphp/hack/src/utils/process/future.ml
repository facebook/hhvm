(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type verbose_error = {
  message: string;
  stack: Utils.callstack;
  environment: string option;
}
[@@deriving show]

(* First class module abstract signature for external errors *)
module type Error = sig
  type config

  type t

  (* Create an error object *)
  val create : config -> t

  (* Callback handler that converts error object to string *)
  val to_string : t -> string

  (* Callback handler that converts error object to verbose_error *)
  val to_string_verbose : t -> verbose_error
end

(* Wrapper module the bundles Error object with its handler.
    This uses the technique/pattern as "A Query-Handling Framework"
    example in RealWorld Ocaml book *)
module type Error_instance = sig
  module Error : Error

  val this : Error.t
end

(* Future error can be categorized into interanl errors and external
   environment specific errors. For external errors, the environment
   specific handlers will be used to convert the error into string *)
type error =
  | External of (module Error_instance)
  | Internal_Continuation_raised of Exception.t
  | Internal_timeout of Exception.t

type 'value get_value = timeout:int -> ('value, (module Error_instance)) result

type is_value_ready = unit -> bool

type 'value status =
  | Complete_with_result of ('value, error) result
  | In_progress of { age: float }

exception Future_failure of error

type 'value delayed = {
  (* Number of times it has been tapped by "is_ready" or "check_status". *)
  tapped: int;
  (* Number of times remaining to be tapped before it is ready. *)
  remaining: int;
  value: 'value;
}

type 'value incomplete = {
  get_value: 'value get_value;
  deadline: float option;
  is_value_ready: is_value_ready;
}

type 'value t = 'value promise ref * creation_time

and 'value promise =
  | Complete : 'value -> 'value promise
  | Complete_but_failed of error
  (* Delayed is useful for deterministic testing. Must be tapped by "is ready" or
   * "check_status" the remaining number of times before it is ready. *)
  | Delayed : 'value delayed -> 'value promise
  (* A future formed from two underlying futures. Calling "get" blocks until
   * both underlying are ready. *)
  | Merged :
      'a t
      * 'b t
      * (('a, error) result -> ('b, error) result -> ('value, error) result)
      -> 'value promise
  (* The future's success is bound to another future producer; it's a chain
      of futures that have to execute serially in order, as opposed to
      the list of Merged futures above that have to execute in parallel. *)
  | Bound :
      'value t * (('value, error) result -> 'next_value t)
      -> 'next_value promise
  (* The future is not yet fulfilled. *)
  | Incomplete of 'value incomplete

and creation_time = float

let equal (_f : 'a -> 'a -> bool) (x : 'a t) (y : 'a t) = Poly.equal x y

(* 30 seconds *)
let default_timeout = 30

let create_error_instance (type a) (module E : Error with type config = a) err =
  ( module struct
    module Error = E

    let this = E.create err
  end : Error_instance )

(* Provide a default error implementation *)
module DefaultError = struct
  type config = string

  type t = string

  let create config = config

  let to_string (error : t) : string = error

  let to_string_verbose (error : t) : verbose_error =
    { message = error; environment = None; stack = Utils.Callstack error }
end

(* Helper function to create a default error instance *)
let create_default_error_instance (error : string) =
  create_error_instance (module DefaultError) error

(* Given an optional deadline, constructs a timeout time span, in seconds,
    relative to the current time (dealine - now). If the current time is past
    the deadline, the timeout is 0. If the deadline is not specified,
    the max timeout value is returned. *)
let timeout_of_deadline deadline ~max_timeout =
  match deadline with
  | Some deadline ->
    let time_left = deadline -. Unix.gettimeofday () in
    if Float.(time_left > 0.0) then
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

let make ?(timeout : int option) (get_value, is_value_ready) : 'value t =
  let deadline =
    deadline_of_timeout ~timeout ~start_time:(Unix.gettimeofday ())
  in
  ( ref (Incomplete { get_value; deadline; is_value_ready }),
    Unix.gettimeofday () )

let of_value (value : 'value) : 'value t =
  (ref @@ Complete value, Unix.gettimeofday ())

let of_error (e : string) =
  try failwith e
  with e ->
    ( ref
      @@ Complete_but_failed (Internal_Continuation_raised (Exception.wrap e)),
      Unix.gettimeofday () )

let delayed_value ~(delays : int) (value : 'value) : 'value t =
  ( ref @@ Delayed { tapped = 0; remaining = delays; value },
    Unix.gettimeofday () )

let error_to_exn e = Future_failure e

let error_to_string err : string =
  match err with
  | Internal_Continuation_raised e ->
    Printf.sprintf "Continuation_raised(%s)" (Exception.get_ctor_string e)
  | Internal_timeout e ->
    Printf.sprintf "Timed_out(%s)" (Exception.get_ctor_string e)
  | External err ->
    let module I = (val err : Error_instance) in
    I.Error.to_string I.this

let error_to_string_verbose err : verbose_error =
  match err with
  | Internal_Continuation_raised ex ->
    let stack = Exception.get_backtrace_string ex |> Exception.clean_stack in
    {
      message =
        Printf.sprintf
          "Continuation failure - %s"
          (Exception.get_ctor_string ex);
      environment = None;
      stack = Utils.Callstack stack;
    }
  | Internal_timeout ex ->
    {
      message = Exception.to_string ex;
      stack = Utils.Callstack (Exception.get_backtrace_string ex);
      environment = None;
    }
  | External err ->
    let module I = (val err : Error_instance) in
    I.Error.to_string_verbose I.this

(* Must explicitly make recursive functions polymorphic. *)
let rec get : 'value. ?timeout:int -> 'value t -> ('value, error) result =
 fun ?(timeout = default_timeout) (promise, _) ->
  match !promise with
  | Complete v -> Ok v
  | Complete_but_failed e -> Error e
  | Delayed { value; remaining; _ } when remaining <= 0 -> Ok value
  | Delayed _ ->
    let error =
      Internal_timeout
        (Exception.wrap_unraised (Failure "Delayed value not ready yet"))
    in
    Error error
  | Merged (a_future, b_future, handler) ->
    let start_t = Unix.gettimeofday () in
    let a_result = get ~timeout a_future in
    let consumed_t = int_of_float @@ (Unix.gettimeofday () -. start_t) in
    let timeout = timeout - consumed_t in
    let b_result = get ~timeout b_future in
    (* NB: We don't need to cache the result of running the handler because
     * underlying Futures a and b have the values cached internally. So
     * subsequent calls to "get" on this Merged Future will just re-run
     * the handler on the cached result. *)
    handler a_result b_result
  | Bound (curr_future, next_producer) ->
    let start_t = Unix.gettimeofday () in
    let curr_result = get ~timeout curr_future in
    let consumed_t = int_of_float @@ (Unix.gettimeofday () -. start_t) in
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
        promise := Complete_but_failed (Internal_Continuation_raised e);
        Error (Internal_Continuation_raised e)
    end
  | Incomplete { get_value; deadline; is_value_ready = _ } ->
    let timeout = timeout_of_deadline deadline ~max_timeout:timeout in
    let result = get_value ~timeout in
    (match result with
    | Ok res ->
      promise := Complete res;
      Ok res
    | Error err ->
      let error = External err in
      promise := Complete_but_failed error;
      Error error)

let get_exn ?timeout (future : 'value t) =
  get ?timeout future |> Result.map_error ~f:error_to_exn |> Result.ok_exn

(* Must explicitly make recursive functions polymorphic. *)
let rec is_ready : 'value. 'value t -> bool =
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
        promise := Complete_but_failed (Internal_Continuation_raised e);
        true
    end else
      false
  | Incomplete { get_value = _; deadline; is_value_ready } ->
    (* Note: if the promise's own deadline is not set, we allow the caller
        to call is_ready as long as they wish, without timing out *)
    let timeout = timeout_of_deadline deadline ~max_timeout:1 in
    if timeout > 0 then
      is_value_ready ()
    else
      (* E.g., we timed out *)
      true

let merge_status
    (status_a : 'a status)
    (status_b : 'b status)
    (handler :
      ('a, error) result -> ('b, error) result -> ('value, error) result) :
    'value status =
  match (status_a, status_b) with
  | (Complete_with_result result_a, Complete_with_result result_b) ->
    Complete_with_result (handler result_a result_b)
  | (In_progress { age }, In_progress { age = age_b }) when Float.(age > age_b)
    ->
    In_progress { age }
  | (In_progress { age }, _)
  | (_, In_progress { age }) ->
    In_progress { age }

let start_t : 'value. 'value t -> float = (fun (_, time) -> time)

let merge
    (future_a : 'a t)
    (future_b : 'b t)
    (handler :
      ('a, error) result -> ('b, error) result -> ('value, error) result) :
    'value t =
  ( ref @@ Merged (future_a, future_b, handler),
    Float.min (start_t future_a) (start_t future_b) )

let make_continue
    (first : 'first t) (next_producer : ('first, error) result -> 'next t) :
    'next_value t =
  (ref @@ Bound (first, next_producer), start_t first)

let continue_with_future
    (future : 'value t) (continuation_handler : 'value -> 'next_value t) :
    'next_value t =
  let continuation_handler (result : ('value, error) result) =
    match result with
    | Ok value -> continuation_handler value
    | Error error -> (ref @@ Complete_but_failed error, start_t future)
  in
  make_continue future continuation_handler

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
  | Incomplete { get_value; is_value_ready; _ } ->
    let deadline = deadline_of_timeout ~timeout:(Some timeout) ~start_time in
    (ref (Incomplete { get_value; deadline; is_value_ready }), start_time)

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
      let age = Unix.gettimeofday () -. start_t in
      In_progress { age }
  | Incomplete { get_value = _; deadline; is_value_ready } ->
    (* Note: if the promise's own deadline is not set, we allow the caller
        to call check_status as long as they wish, without timing out *)
    let timeout = timeout_of_deadline deadline ~max_timeout:1 in
    if is_value_ready () || timeout <= 0 then
      Complete_with_result (get ~timeout (promise, start_t))
    else
      let age = Unix.gettimeofday () -. start_t in
      In_progress { age }

(* Necessary to avoid a cyclic type definition error. *)
type 'value future = 'value t

module Promise = struct
  type 'value t = 'value future

  let return = of_value

  let map = continue_with

  let bind = continue_with_future

  let both a b =
    merge a b @@ fun a b ->
    match (a, b) with
    | (Error e, _)
    | (_, Error e) ->
      Error e
    | (Ok a, Ok b) -> Ok (a, b)
end

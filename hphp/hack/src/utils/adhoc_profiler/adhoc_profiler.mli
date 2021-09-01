(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This module allows to instrument code that we want to profile.
    The user is free to decide which calls or portion of code they
    want to see appear in the profile.

    Here's an example of usage:
    ```
    let f () =
      Adhoc_profiler.create ~name:"test1" @@ fun profiler ->
      Time.advance 1.2;
      let (profiler, ()) =
        Adhoc_profiler.count_leaf ~name:"step1" profiler @@ fun () ->
        Time.advance 0.3
      in
      Time.advance 0.1;
      let (profiler, ()) =
        Adhoc_profiler.count ~name:"step2" profiler @@ fun profiler ->
        Time.advance 0.7;
        let (profiler, ()) =
          Adhoc_profiler.count_leaf ~name:"step2.1" profiler @@ fun () ->
          Time.advance 1.0
        in
        let (profiler, ()) =
          Adhoc_profiler.count_leaf ~name:"step2.2" profiler @@ fun () ->
          Time.advance 2.0
        in
        (profiler, ())
      in
      (profiler, ())
    ```

    And here's the kind of profile it produces:
    ```
    test1                             2           10.600 sec   100.00%
      step1                             2           0.600 sec   5.66%
      step2                             2           7.400 sec   69.81%
        step2.1                           2           2.000 sec   18.87%
        step2.2                           2           4.000 sec   37.74%
    ```
     *)

type seconds = float

type t

(** Create a profiler to be threaded through code to profile. *)
val create : name:string -> (t -> 'result) -> 'result

(** Create a sub-node in the profiling tree. *)
val count : t -> name:string -> (t -> 'result) -> 'result

(** Create a leaf in the profiling tree. *)
val count_leaf : t -> name:string -> (unit -> 'result) -> 'result

(** Use this to wrap a function taking a profiler as parameter which you don't want to profile. *)
val without_profiling : (prof:t -> 'result) -> 'result

(** Get the accumulated profiling data and reset it. *)
val get_and_reset : unit -> t

(** Print profiling data. *)
val to_string : t -> string

module Test : sig
  val set_time_getter : (unit -> seconds) -> unit
end

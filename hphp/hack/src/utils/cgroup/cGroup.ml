(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Unix = Caml_unix
module Sys = Stdlib.Sys
open Result.Monad_infix

let spf = Printf.sprintf

(* Little helper module to help memoize things. Probably could be pulled out into its own module
 * at some point *)
module Memoize : sig
  val forever : f:(unit -> 'a) -> unit -> 'a

  val until : seconds:float -> f:(unit -> 'a) -> unit -> 'a
end = struct
  let forever ~f =
    let memoized_result = ref None in
    fun () ->
      match !memoized_result with
      | None ->
        let result = f () in
        memoized_result := Some result;
        result
      | Some result -> result

  let until ~seconds ~f =
    let memoized_result = ref None in
    let fetch () =
      let result = f () in
      memoized_result := Some (Unix.gettimeofday () +. seconds, result);
      result
    in
    fun () ->
      match !memoized_result with
      | Some (good_until, result) when Float.(Unix.gettimeofday () < good_until)
        ->
        result
      | _ -> fetch ()
end

(* I've never seen cgroup mounted elsewhere, so it's probably fine to hardcode this for now *)
let cgroup_dir = "/sys/fs/cgroup"

let assert_is_using_cgroup_v2 =
  Memoize.forever ~f:(fun () ->
      if Sys.file_exists cgroup_dir then
        (* /sys/fs/cgroup/memory exists for cgroup v1 but not v2. It's an easy way to tell the
         * difference between versions *)
        if Sys.file_exists (spf "%s/memory" cgroup_dir) then
          Error (spf "cgroup v1 is mounted at %s. We need v2" cgroup_dir)
        else
          Ok ()
      else
        Error (spf "%s doesn't exist" cgroup_dir))

(* I don't really expect us to switch cgroups often, but let's only cache for 5 seconds *)
let get_cgroup_name =
  Memoize.until ~seconds:5.0 ~f:(fun () ->
      ProcFS.first_cgroup_for_pid (Unix.getpid ()))

type stats = {
  memory_current: int;
  memory_swap_current: int;
  total: int;
  anon: int;
  shmem: int;
  file: int;
  cgroup_name: string;
}

(* Some cgroup files contain only a single integer *)
let read_single_number_file path =
  try Ok (Sys_utils.cat path |> String.strip |> int_of_string) with
  | Failure _
  | Sys_error _ ->
    Error "Failed to parse memory.current"

let parse_stat stat_contents =
  let stats =
    String.split stat_contents ~on:'\n'
    |> List.fold_left ~init:SMap.empty ~f:(fun stats line ->
           match String.split line ~on:' ' with
           | [key; raw_stat] ->
             int_of_string_opt raw_stat
             |> Option.value_map ~default:stats ~f:(fun stat ->
                    SMap.add key stat stats)
           | _ -> stats)
  in
  let get key =
    match SMap.find_opt key stats with
    | Some stat -> Ok stat
    | None -> Error (spf "Failed to find %S in memory.stat" key)
  in
  get "anon" >>= fun anon ->
  get "file" >>= fun file ->
  get "shmem" >>| fun shmem ->
  (* In `memory.stat` the `file` stat includes `shmem` *)
  (anon, file - shmem, shmem)

let get_stats_for_cgroup (cgroup_name : string) : (stats, string) result =
  (* cgroup_name starts with a /, like /my_cgroup *)
  let dir = spf "%s%s" cgroup_dir cgroup_name in
  let memory_current_result =
    read_single_number_file (Filename.concat dir "memory.current")
  and memory_swap_current_result =
    read_single_number_file (Filename.concat dir "memory.swap.current")
  and stat_contents_result =
    try Ok (Sys_utils.cat (Filename.concat dir "memory.stat")) with
    | Failure _
    | Sys_error _ ->
      Error "Failed to parse memory.stat"
  in
  memory_current_result >>= fun memory_current ->
  memory_swap_current_result >>= fun memory_swap_current ->
  stat_contents_result >>= fun stat_contents ->
  parse_stat stat_contents >>= fun (anon, file, shmem) ->
  let total = memory_current + memory_swap_current in
  Ok
    {
      total;
      memory_current;
      memory_swap_current;
      anon;
      file;
      shmem;
      cgroup_name;
    }

let get_stats () =
  assert_is_using_cgroup_v2 () >>= get_cgroup_name >>= get_stats_for_cgroup

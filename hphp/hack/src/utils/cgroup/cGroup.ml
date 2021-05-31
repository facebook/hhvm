(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Unix = Caml_unix
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
  total: int;
  (* The total physical memory for the cgroup *)
  total_swap: int;
  (* The total amount of anonymous memory paged out to swap *)

  (* anon, file, and shmem are disjoint. If you add in the memory that the kernel uses, they should
   * sum roughly to `total` *)
  anon: int;
  (* The amount of physical anonymous memory not used for shared memory *)
  shmem: int;
  (* The amount of physical anonymous memory being used as shared memory *)
  file: int; (* The amount of physical memory which is not anonymous *)
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
  let total_result =
    read_single_number_file (Filename.concat dir "memory.current")
  and total_swap_result =
    read_single_number_file (Filename.concat dir "memory.swap.current")
  and stat_contents_result =
    try Ok (Sys_utils.cat (Filename.concat dir "memory.stat")) with
    | Failure _
    | Sys_error _ ->
      Error "Failed to parse memory.stat"
  in
  total_result >>= fun total ->
  total_swap_result >>= fun total_swap ->
  stat_contents_result >>= fun stat_contents ->
  parse_stat stat_contents >>= fun (anon, file, shmem) ->
  Ok { total; total_swap; anon; file; shmem }

let get_stats () =
  assert_is_using_cgroup_v2 () >>= get_cgroup_name >>= get_stats_for_cgroup

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *)

type stats = {
  total: int;
      (** cgroup/memory.current - the total physical memory for the cgroup *)
  total_swap: int;
      (** cgroup/memory.swap.current - the total amount of anonymous memory paged out to swap.
      Note that anon, file, and shmem are disjoint. If you add in the memory that the kernel uses,
      they should sum roughly to `total` *)
  anon: int;
      (** cgroup/memory.stat:anon - the amount of physical anonymous memory not used for shared memory *)
  shmem: int;
      (** cgroup/memory.stat:shmem - the amount of physical anonymous memory being used as shared memory *)
  file: int;
      (** cgroup/memory.stat:file-shmem - the amount of physical memory which is not anonymous *)
  cgroup_name: string;
      (** the cgroup we queried, obtained dyanmically based on our pid *)
}

(** Reads the current PID, figures out which cgroup it's in (if cgroups are available),
and queries it. *)
val get_stats : unit -> (stats, string) result

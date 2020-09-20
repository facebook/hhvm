(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external realpath : string -> string option = "hh_realpath"

external is_nfs : string -> bool = "hh_is_nfs"

external is_apple_os : unit -> bool = "hh_sysinfo_is_apple_os"

(** Option type intead of exception throwing. *)
val get_env : string -> string option

val getenv_user : unit -> string option

val getenv_home : unit -> string option

val getenv_term : unit -> string option

val path_sep : string

val null_path : string

val temp_dir_name : string

val getenv_path : unit -> string option

val open_in_no_fail : string -> in_channel

val open_in_bin_no_fail : string -> in_channel

val close_in_no_fail : string -> in_channel -> unit

val open_out_no_fail : string -> out_channel

val open_out_bin_no_fail : string -> out_channel

val close_out_no_fail : string -> out_channel -> unit

val file_exists : string -> bool

val cat : string -> string

val cat_or_failed : string -> string option

val cat_no_fail : string -> string

val nl_regexp : Str.regexp

val split_lines : string -> string list

(** Returns true if substring occurs somewhere inside str. *)
val string_contains : string -> string -> bool

val exec_read : string -> string

val exec_read_lines : ?reverse:bool -> string -> string Hh_core.List.t

val collect_paths : (string -> bool) -> string -> string Hh_core.List.t

(**
 * Sometimes the user wants to pass a list of paths on the command-line.
 * However, we have enough files in the codebase that sometimes that list
 * exceeds the maximum number of arguments that can be passed on the
 * command-line. To work around this, we can use the convention that some Unix
 * tools use: a `@` before a path name represents a file that should be read
 * to get the necessary information (in this case, containing a list of files,
 * one per line).
 *)
val parse_path_list : string list -> string list

val rm_dir_tree : ?skip_mocking:bool -> string -> unit

val restart : unit -> 'a

val logname_impl : unit -> string

val logname : unit -> string

val get_primary_owner : unit -> string

val with_umask : int -> (unit -> 'a) -> 'a

val read_stdin_to_string : unit -> string

val read_all : ?buf_size:int -> in_channel -> string

(** Like Python's os.path.expanduser, though probably doesn't cover some
cases. Roughly follow's bash's tilde expansion:
http://www.gnu.org/software/bash/manual/html_node/Tilde-Expansion.html

    ~/foo -> /home/bob/foo if $HOME = "/home/bob"
    ~joe/foo -> /home/joe/foo if joe's home is /home/joe
*)
val expanduser : string -> string

(** Turns out it's surprisingly complex to figure out the path to the current
executable, which we need in order to extract its embedded libraries. If
argv[0] is a path, then we can use that; sometimes it's just the exe name, so
we have to search $PATH for it the same way shells do. for example:
https://www.gnu.org/software/bash/manual/html_node/Command-Search-and-Execution.html

There are other options which might be more reliable when they exist, like
using the `_` env var set by bash, or /proc/self/exe on Linux, but they are
not portable. *)
val executable_path : unit -> string

val lines_of_in_channel : in_channel -> string Hh_core.List.t

val lines_of_file : string -> string Hh_core.List.t

val read_file : string -> bytes

val write_file : file:string -> string -> unit

val append_file : file:string -> string -> unit

val write_strings_to_file : file:string -> string list -> unit

val mkdir_p : ?skip_mocking:bool -> string -> unit

val mkdir_no_fail : string -> unit

val unlink_no_fail : string -> unit

val readlink_no_fail : string -> string

val filemtime : string -> float

external lutimes : string -> unit = "hh_lutimes"

type touch_mode =
  | Touch_existing of { follow_symlinks: bool }
  | Touch_existing_or_create_new of {
      mkdir_if_new: bool;
      perm_if_new: Unix.file_perm;
    }

val touch : touch_mode -> string -> unit

val try_touch : touch_mode -> string -> unit

val splitext : string -> string * string

val is_test_mode : unit -> bool

val sleep : seconds:float -> unit

val symlink : string -> string -> unit

(** Creates a symlink at `<dir>/<linkname.ext>` to
`<dir>/<pluralized ext>/<linkname>-<timestamp>.<ext>` *)
val make_link_of_timestamped : string -> string

val setsid : unit -> int

val set_signal : int -> Sys.signal_behavior -> unit

val signal : int -> Sys.signal_behavior -> unit

external get_total_ram : unit -> int = "hh_sysinfo_totalram"

external uptime : unit -> int = "hh_sysinfo_uptime"

external nproc : unit -> int = "nproc"

val total_ram : int

val nbr_procs : int

external set_priorities : cpu_priority:int -> io_priority:int -> unit
  = "hh_set_priorities"

external pid_of_handle : int -> int = "pid_of_handle"

external handle_of_pid_for_termination : int -> int
  = "handle_of_pid_for_termination"

val terminate_process : int -> unit

val lstat : string -> Unix.stats

val normalize_filename_dir_sep : string -> string

val name_of_signal : int -> string

(** The units for each of these fields is seconds, similar to Unix.times().
`cpu_info` and `processor_info` are constructed by c code (see
`processor_info.c`) so be very careful modifying these types! *)
type cpu_info = {
  cpu_user: float;
  cpu_nice_user: float;
  cpu_system: float;
  cpu_idle: float;
}

type processor_info = {
  proc_totals: cpu_info;
  proc_per_cpu: cpu_info array;
}

external processor_info : unit -> processor_info = "hh_processor_info"

(** We implement timers using sigalarm which means selects can be
interrupted. This is a wrapper around EINTR which continues the select if it
gets interrupted by a signal *)
val select_non_intr :
  Unix.file_descr list ->
  Unix.file_descr list ->
  Unix.file_descr list ->
  float ->
  Unix.file_descr list * Unix.file_descr list * Unix.file_descr list

(** Flow uses lwt, which installs a sigchld handler. So the old pattern of
fork & waitpid will hit an EINTR when the forked process dies and the parent
gets a sigchld signal.

Note: this is only a problem if you're not using the WNOHANG flag, since
EINTR isn't thrown for WNOHANG *)
val waitpid_non_intr : Unix.wait_flag list -> int -> int * Unix.process_status

val check_dmesg_for_oom : int -> string -> bool

type rusage = {
  ru_maxrss: int;
  ru_ixrss: int;
  ru_idrss: int;
  ru_isrss: int;
  ru_minflt: int;
  ru_majflt: int;
  ru_nswap: int;
  ru_inblock: int;
  ru_oublock: int;
  ru_msgsnd: int;
  ru_msgrcv: int;
  ru_nsignals: int;
  ru_nvcsw: int;
  ru_nivcsw: int;
}

external getrusage : unit -> rusage = "hh_getrusage"

external start_gc_profiling : unit -> unit = "hh_start_gc_profiling" [@@noalloc]

external get_gc_time : unit -> float * float = "hh_get_gc_time"

module For_test : sig
  val find_oom_in_dmesg_output : int -> string -> string Hh_core.List.t -> bool
end

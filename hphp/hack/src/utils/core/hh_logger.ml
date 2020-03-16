(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let timestamp_string () =
  Unix.(
    let tm = localtime (time ()) in
    let ms = int_of_float (gettimeofday () *. 1000.) mod 1000 in
    let year = tm.tm_year + 1900 in
    Printf.sprintf
      "[%d-%02d-%02d %02d:%02d:%02d.%03d]"
      year
      (tm.tm_mon + 1)
      tm.tm_mday
      tm.tm_hour
      tm.tm_min
      tm.tm_sec
      ms)

(* We might want to log to both stderr and a file. Shelling out to tee isn't cross-platform.
 * We could dup2 stderr to a pipe and have a child process write to both original stderr and the
 * file, but that's kind of overkill. This is good enough *)
let dupe_log : out_channel option ref = ref None

let set_log _filename fd = dupe_log := Some fd

let id : string option ref = ref None

let set_id passed_id = id := Some passed_id

let id_string () =
  match !id with
  | None -> ""
  | Some id -> Printf.sprintf "[%s] " id

type passes = {
  passes_file: bool;
  passes_stderr: bool;
}

let print_with_newline_internal ~passes ?exn fmt =
  let print_raw ?exn s =
    let exn_str =
      Option.value_map exn ~default:"" ~f:(fun exn ->
          let bt =
            String_utils.indent 8
            @@ String.trim
            @@ Exception.get_backtrace_string exn
          in
          let bt =
            if bt = "" then
              ""
            else
              "\n    Backtrace:\n" ^ bt
          in
          Printf.sprintf
            "\n    Exception: %s%s"
            (Exception.get_ctor_string exn)
            bt)
    in
    let time = timestamp_string () in
    let id_str = id_string () in
    begin
      match (!dupe_log, passes.passes_file) with
      | (Some dupe_log_oc, true) ->
        Printf.fprintf dupe_log_oc "%s %s%s%s\n%!" time id_str s exn_str
      | (_, _) -> ()
    end;
    if passes.passes_stderr then
      Printf.eprintf "%s %s%s%s\n%!" time id_str s exn_str
  in
  Printf.ksprintf (print_raw ?exn) fmt

let print_with_newline ?exn fmt =
  print_with_newline_internal
    ~passes:{ passes_file = true; passes_stderr = true }
    ?exn
    fmt

let print_duration name t =
  let t2 = Unix.gettimeofday () in
  print_with_newline "%s: %f" name (t2 -. t);
  t2

let exc ?(prefix : string = "") ~(stack : string) (e : exn) : unit =
  print_with_newline "%s%s\n%s" prefix (Printexc.to_string e) stack

let exception_ ?(prefix : string = "") (e : Exception.t) : unit =
  exc ~prefix ~stack:(Exception.get_backtrace_string e) (Exception.unwrap e)

module Level : sig
  type t =
    | Off
    | Fatal
    | Error
    | Warn
    | Info
    | Debug
  [@@deriving enum]

  val of_enum_string : string -> t option

  val to_enum_string : t -> string

  val min_level_file : unit -> t

  val min_level_stderr : unit -> t

  val set_min_level : t -> unit

  val set_min_level_file : t -> unit

  val set_min_level_stderr : t -> unit

  val passes_min_level : t -> bool

  val log :
    t ->
    ?exn:Exception.t ->
    ('a, unit, string, string, string, unit) format6 ->
    'a

  val log_duration : t -> string -> float -> float
end = struct
  type t =
    | Off [@value 6]
    | Fatal [@value 5]
    | Error [@value 4]
    | Warn [@value 3]
    | Info [@value 2]
    | Debug [@value 1]
  [@@deriving enum]

  let to_enum_string = function
    | Off -> "off"
    | Fatal -> "fatal"
    | Error -> "error"
    | Warn -> "warn"
    | Info -> "info"
    | Debug -> "debug"

  let of_enum_string = function
    | "off" -> Some Off
    | "fatal" -> Some Fatal
    | "error" -> Some Error
    | "warn" -> Some Warn
    | "info" -> Some Info
    | "debug" -> Some Debug
    | _ -> None

  let min_level_file_ref = ref Info

  let min_level_stderr_ref = ref Info

  let set_min_level_file level = min_level_file_ref := level

  let set_min_level_stderr level = min_level_stderr_ref := level

  let set_min_level level =
    set_min_level_file level;
    set_min_level_stderr level;
    ()

  let min_level_file () = !min_level_file_ref

  let min_level_stderr () = !min_level_stderr_ref

  let passes level =
    let ilevel = to_enum level in
    let passes_file = ilevel >= to_enum !min_level_file_ref in
    let passes_stderr = ilevel >= to_enum !min_level_stderr_ref in
    if passes_file || passes_stderr then
      Some { passes_file; passes_stderr }
    else
      None

  let passes_min_level level = passes level |> Option.is_some

  let log level ?exn fmt =
    match passes level with
    | Some passes -> print_with_newline_internal ~passes ?exn fmt
    | None -> Printf.ifprintf () fmt

  let log_duration level name t =
    let t2 = Unix.gettimeofday () in
    begin
      match passes level with
      | Some passes ->
        print_with_newline_internal ~passes "%s: %f" name (t2 -. t)
      | None -> ()
    end;
    t2
end

(* Default log instructions to INFO level *)
let log ?(lvl = Level.Info) fmt = Level.log lvl fmt

let log_duration ?(lvl = Level.Info) fmt t = Level.log_duration lvl fmt t

let fatal ?exn fmt = Level.log Level.Fatal ?exn fmt

let error ?exn fmt = Level.log Level.Error ?exn fmt

let warn ?exn fmt = Level.log Level.Warn ?exn fmt

let info ?exn fmt = Level.log Level.Info ?exn fmt

let debug ?exn fmt = Level.log Level.Debug ?exn fmt

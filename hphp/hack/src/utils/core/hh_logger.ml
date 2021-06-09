(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* We might want to log to both stderr and a file. Shelling out to tee isn't cross-platform.
 * We could dup2 stderr to a pipe and have a child process write to both original stderr and the
 * file, but that's kind of overkill. This is good enough *)
let dupe_log : Out_channel.t option ref = ref None

let set_log filename =
  Option.iter !dupe_log ~f:(fun fd -> Out_channel.close fd);
  dupe_log := Some (Out_channel.create filename ~append:true);
  ()

let id : string option ref = ref None

let set_id passed_id = id := Some passed_id

let id_string () =
  match !id with
  | None -> ""
  | Some id -> Printf.sprintf "[%s] " id

let category_string category =
  match category with
  | None -> ""
  | Some category -> Printf.sprintf "[%s] " category

type passes = {
  passes_file: bool;
  passes_stderr: bool;
}

let print_with_newline_internal ?category ~passes ?exn fmt =
  let print_raw ?exn s =
    let exn_str =
      Option.value_map exn ~default:"" ~f:(fun exn ->
          let bt =
            String_utils.indent 8
            @@ String.strip
            @@ Exception.get_backtrace_string exn
          in
          let bt =
            if String.equal bt "" then
              ""
            else
              "\n    Backtrace:\n" ^ bt
          in
          Printf.sprintf
            "\n    Exception: %s%s"
            (Exception.get_ctor_string exn)
            bt)
    in
    let time = Utils.timestring (Unix.gettimeofday ()) in
    let id_str = id_string () in
    let category_str = category_string category in
    begin
      match (!dupe_log, passes.passes_file) with
      | (Some dupe_log_oc, true) ->
        Printf.fprintf
          dupe_log_oc
          "%s %s%s%s%s\n%!"
          time
          id_str
          category_str
          s
          exn_str
      | (_, _) -> ()
    end;
    if passes.passes_stderr then
      Printf.eprintf "%s %s%s%s%s\n%!" time id_str category_str s exn_str
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
  print_with_newline "%s%s\n%s" prefix (Exn.to_string e) stack

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

  val set_categories : string list -> unit

  val log :
    t ->
    ?category:string ->
    ?exn:Exception.t ->
    ('a, unit, string, string, string, unit) format6 ->
    'a

  val log_lazy :
    t -> ?category:string -> ?exn:Exception.t -> string lazy_t -> unit

  val log_duration : t -> ?category:string -> string -> float -> float
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

  let categories_ref = ref SSet.empty

  let set_min_level_file level = min_level_file_ref := level

  let set_min_level_stderr level = min_level_stderr_ref := level

  let set_min_level level =
    set_min_level_file level;
    set_min_level_stderr level;
    ()

  let min_level_file () = !min_level_file_ref

  let min_level_stderr () = !min_level_stderr_ref

  let set_categories categories = categories_ref := SSet.of_list categories

  let passes level ~category =
    let passes_category =
      match category with
      | None -> true
      | Some category -> SSet.mem category !categories_ref
    in
    if not passes_category then
      None
    else
      let ilevel = to_enum level in
      let passes_file = ilevel >= to_enum !min_level_file_ref in
      let passes_stderr = ilevel >= to_enum !min_level_stderr_ref in
      if passes_file || passes_stderr then
        Some { passes_file; passes_stderr }
      else
        None

  let passes_min_level level = passes level ~category:None |> Option.is_some

  let log level ?category ?exn fmt =
    match passes level ~category with
    | Some passes -> print_with_newline_internal ?category ~passes ?exn fmt
    | None -> Printf.ifprintf () fmt

  let log_lazy level ?category ?exn s =
    match passes level ~category with
    | Some passes ->
      print_with_newline_internal ?category ~passes ?exn "%s" (Lazy.force s)
    | None -> ()

  let log_duration level ?category name t =
    let t2 = Unix.gettimeofday () in
    begin
      match passes level ~category with
      | Some passes ->
        print_with_newline_internal ?category ~passes "%s: %f" name (t2 -. t)
      | None -> ()
    end;
    t2
end

(* Default log instructions to INFO level *)
let log ?(lvl = Level.Info) ?category fmt = Level.log lvl ?category fmt

let log_lazy ?(lvl = Level.Info) ?category str =
  Level.log_lazy lvl ?category str

let log_duration ?(lvl = Level.Info) ?category fmt t =
  Level.log_duration lvl ?category fmt t

let fatal ?category ?exn fmt = Level.log Level.Fatal ?category ?exn fmt

let error ?category ?exn fmt = Level.log Level.Error ?category ?exn fmt

let warn ?category ?exn fmt = Level.log Level.Warn ?category ?exn fmt

let info ?category ?exn fmt = Level.log Level.Info ?category ?exn fmt

let debug ?category ?exn fmt = Level.log Level.Debug ?category ?exn fmt

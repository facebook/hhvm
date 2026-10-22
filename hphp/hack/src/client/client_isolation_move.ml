(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Dir_move = struct
  (** What a move has done to the disk, newest first, so that undoing it is
      reading it from the front. *)
  type step =
    | Made_dir of string
    | Renamed of {
        from: string;
        into: string;
      }

  type t = step list

  (** The directories above [target] that do not exist yet, outermost first. *)
  let missing_parents target =
    let rec go dir missing =
      if String.is_empty dir || String.equal dir "." || Disk.file_exists dir
      then
        missing
      else
        go (Filename.dirname dir) (dir :: missing)
    in
    go (Filename.dirname target) []

  let make_dir journal path =
    Unix.mkdir path 0o755;
    journal := Made_dir path :: !journal

  let exists path =
    match Unix.lstat path with
    | _ -> true
    | exception Unix.Unix_error (Unix.ENOENT, _, _) -> false

  (** A step whose result is already there is skipped. *)
  let undo_step = function
    | Renamed { from; into } -> if exists into then Unix.rename into from
    | Made_dir path ->
      (try Unix.rmdir path with
      | Unix.Unix_error ((Unix.ENOTEMPTY | Unix.EEXIST | Unix.ENOENT), _, _) ->
        ())

  (** Every step is tried, so one that fails does not leave the rest undone, and
      the failures are then reported together. *)
  let undo steps =
    let failures =
      List.filter_map steps ~f:(fun step ->
          try
            undo_step step;
            None
          with
          | exn ->
            Some
              (match step with
              | Made_dir path ->
                Printf.sprintf "%s: %s" path (Exn.to_string exn)
              | Renamed { from; into } ->
                Printf.sprintf "%s to %s: %s" into from (Exn.to_string exn)))
    in
    if not (List.is_empty failures) then
      failwith
        (Printf.sprintf
           "could not undo the move completely:\n  %s"
           (String.concat ~sep:"\n  " failures))

  let move ~source ~target =
    let journal = ref [] in
    match
      List.iter (missing_parents target) ~f:(make_dir journal);
      Unix.rename source target;
      journal := Renamed { from = source; into = target } :: !journal
    with
    | () -> Ok !journal
    | exception exn ->
      let reason = Exn.to_string exn in
      (match undo !journal with
      | () -> Error reason
      | exception Failure undo_failure ->
        Error (Printf.sprintf "%s; %s" reason undo_failure))
end

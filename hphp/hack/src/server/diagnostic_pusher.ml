(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix
open Reordered_argument_collections

type error = Errors.error [@@deriving show]

type seconds_since_epoch = float

type phase = PhaseSingleton [@@deriving eq, show]

module PhaseMap = struct
  include Reordered_argument_map (WrappedMap.Make (struct
    type t = phase

    let rank = function
      | PhaseSingleton -> 4

    let compare x y = rank x - rank y
  end))

  let pp pp_data = make_pp pp_phase pp_data

  let show pp_data x = Format.asprintf "%a" (pp pp_data) x
end

module FileMap = Relative_path.Map

(** A 2D map from files to phases to errors, with the usual map helpers. *)
module ErrorMap = struct
  type errors = error list [@@deriving show]

  let equal_errors err1 err2 =
    let err1 = Errors.ErrorSet.of_list err1 in
    let err2 = Errors.ErrorSet.of_list err2 in
    Errors.ErrorSet.equal err1 err2

  type t = errors PhaseMap.t FileMap.t [@@deriving eq, show]

  let remove map k1 k2 =
    FileMap.find_opt map k1
    >>| (fun inner_map -> PhaseMap.remove inner_map k2)
    |> Option.fold ~init:map ~f:(fun map inner_map ->
           if PhaseMap.is_empty inner_map then
             FileMap.remove map k1
           else
             FileMap.add map ~key:k1 ~data:inner_map)

  (** Set value in map for given file and phase.
      If the file is not already present in the map, initialize its phase map
      with the phase map for that file from [init_phase_map_from] *)
  let set :
      t -> Relative_path.t -> phase -> errors -> init_phase_map_from:t -> t =
   fun map file phase data ~init_phase_map_from:init_map ->
    FileMap.find_opt map file
    |> Option.value
         ~default:
           (FileMap.find_opt init_map file
           |> Option.value ~default:PhaseMap.empty)
    |> PhaseMap.add ~key:phase ~data
    |> fun inner_map -> FileMap.add map ~key:file ~data:inner_map

  let find_opt map k1 k2 =
    FileMap.find_opt map k1 >>= fun inner_map -> PhaseMap.find_opt inner_map k2

  let mem map k1 k2 = Option.is_some @@ find_opt map k1 k2

  let files = FileMap.keys
end

(** This keeps track of the set of errors in the current LSP client and
    the set of errors which still need to be pushed. *)
module ErrorTracker : sig
  type t [@@deriving eq, show]

  val init : t

  (** Inform the tracker of a new batch of errors produced by the server for
      a specific set of [rechecked] files, and
      get the set of errors which need to be pushed.
      This set may or may not contain the new errors, depending on what's in the IDE
      and what was successfully pushed so far. *)
  val get_errors_to_push :
    t ->
    rechecked:Relative_path.Set.t ->
    new_errors:Errors.t ->
    priority_files:Relative_path.Set.t option ->
    t * ServerCommandTypes.diagnostic_errors

  (** Inform the tracker that errors yet to be pushed have been successfully pushed
      to the IDE. *)
  val commit_pushed_errors : t -> t

  (** Reset the tracker for when the previous client has been lost
      and we have got / will get a new one. *)
  val reset_for_new_client : t -> t

  val get_files_with_diagnostics : t -> Relative_path.t list

  (** Module to export internal functions for unit testing. Do not use in production code. *)
  module TestExporter : sig
    val make :
      errors_in_ide:ErrorMap.t ->
      to_push:ErrorMap.t ->
      errors_beyond_limit:ErrorMap.t ->
      t

    val with_error_limit : int -> (unit -> 'result) -> 'result
  end
end = struct
  type t = {
    errors_in_ide: ErrorMap.t;
        (** The set of errors currently in the IDE, based on what has been pushed so far. *)
    to_push: ErrorMap.t;  (** The set of errors yet to be pushed to the IDE. *)
    errors_beyond_limit: ErrorMap.t;
        (** Errors beyond the limit on the number of errors we want to have in the IDE.
            We may be able to send those later. *)
  }
  [@@deriving eq, show]

  (** Limit on the number of errors we want to have in the IDE.
      The IDE may not be very resonsive beyond this limit. *)
  let default_error_limit = 1000

  let error_limit : int ref = ref default_error_limit

  let init =
    {
      errors_in_ide = FileMap.empty;
      to_push = FileMap.empty;
      errors_beyond_limit = FileMap.empty;
    }

  let error_count_in_phase_map : error list PhaseMap.t -> int =
    PhaseMap.fold ~init:0 ~f:(fun _phase errors count ->
        List.length errors + count)

  let error_count_in_file_map : ErrorMap.t -> int =
    FileMap.fold ~init:0 ~f:(fun _file phase_map count ->
        error_count_in_phase_map phase_map + count)

  (** For each file key from [rechecked] and for [phase], update value in [to_push] with value from
      [new_error], possibly removing entry if there are no more errors.
      [errors_in_ide] is used to possibly initialize other phases for a file in [to_push]. *)
  let merge_errors_to_push :
      ErrorMap.t ->
      errors_in_ide:ErrorMap.t ->
      rechecked:Relative_path.Set.t ->
      new_errors:error list FileMap.t ->
      phase:phase ->
      ErrorMap.t =
   fun to_push ~errors_in_ide ~rechecked ~new_errors ~phase ->
    let to_push =
      Relative_path.Set.fold
        rechecked
        ~init:to_push
        ~f:(fun rechecked_file to_push ->
          ErrorMap.remove to_push rechecked_file phase)
    in
    let to_push =
      FileMap.fold new_errors ~init:to_push ~f:(fun file file_errors to_push ->
          if List.is_empty file_errors then
            (* We'll deal with erasing (which we do by sending the empty list) in erase_errors.
             * This should normally not happen, we're just being defensive here. *)
            to_push
          else
            ErrorMap.set
              to_push
              file
              phase
              file_errors
              ~init_phase_map_from:errors_in_ide)
    in
    to_push

  (** Add [file -> phase -> []] entry in the [to_push] map for each file which used to have
      errors but doesn't anymore. *)
  let erase_errors :
      errors_in_ide:ErrorMap.t ->
      rechecked:Relative_path.Set.t ->
      phase:phase ->
      ErrorMap.t ->
      ErrorMap.t =
   fun ~errors_in_ide ~rechecked ~phase to_push ->
    let rechecked_which_had_errors =
      Relative_path.Set.filter rechecked ~f:(fun file ->
          ErrorMap.mem errors_in_ide file phase)
    in
    let to_push =
      Relative_path.Set.fold
        rechecked_which_had_errors
        ~init:to_push
        ~f:(fun rechecked_file to_push ->
          if ErrorMap.mem to_push rechecked_file phase then
            to_push
          else
            ErrorMap.set
              to_push
              rechecked_file
              phase
              []
              ~init_phase_map_from:errors_in_ide)
    in
    to_push

  (** Truncate the list of errors to push so that the total number of errors
      in the IDE does not exceed [error_limit]. This partitions the errors as
      a set of errors we can push now and the remainder of the errors. *)
  let truncate :
      ErrorMap.t ->
      errors_in_ide:ErrorMap.t ->
      priority_files:Relative_path.Set.t option ->
      ErrorMap.t * ErrorMap.t =
   fun to_push_candidates ~errors_in_ide ~priority_files ->
    let error_count_in_ide = error_count_in_file_map errors_in_ide in
    let to_push = FileMap.empty in
    (* First push errors in prority files, regardless of error limit. *)
    let (to_push, to_push_candidates, error_count_in_ide) =
      Relative_path.Set.fold
        (priority_files |> Option.value ~default:Relative_path.Set.empty)
        ~init:(to_push, to_push_candidates, error_count_in_ide)
        ~f:(fun file (to_push, to_push_candidates, error_count_in_ide) ->
          match FileMap.find_opt to_push_candidates file with
          | None -> (to_push, to_push_candidates, error_count_in_ide)
          | Some phase_map ->
            let to_push = FileMap.add to_push ~key:file ~data:phase_map in
            let to_push_candidates = FileMap.remove to_push_candidates file in
            let error_count_in_ide =
              let error_count = error_count_in_phase_map phase_map in
              let error_count_before =
                FileMap.find_opt errors_in_ide file
                >>| error_count_in_phase_map
                |> Option.value ~default:0
              in
              error_count_in_ide + error_count - error_count_before
            in
            (to_push, to_push_candidates, error_count_in_ide))
    in
    (* Then push errors in files which already have errors *)
    let (to_push, to_push_candidates, error_count_in_ide) =
      FileMap.fold
        to_push_candidates
        ~init:(to_push, FileMap.empty, error_count_in_ide)
        ~f:(fun file phase_map (to_push, to_push_candidates, error_count_in_ide)
           ->
          match FileMap.find_opt errors_in_ide file with
          | None ->
            (* Let's consider it again later. *)
            let to_push_candidates =
              FileMap.add to_push_candidates ~key:file ~data:phase_map
            in
            (to_push, to_push_candidates, error_count_in_ide)
          | Some errors_in_ide ->
            let error_count = error_count_in_phase_map phase_map in
            let error_count_before = error_count_in_phase_map errors_in_ide in
            if
              error_count_in_ide + error_count - error_count_before
              <= !error_limit
            then
              let error_count_in_ide =
                error_count_in_ide + error_count - error_count_before
              in
              let to_push = FileMap.add to_push ~key:file ~data:phase_map in
              (to_push, to_push_candidates, error_count_in_ide)
            else
              (* Erase those errors in the IDE, because for each file we want either all errors or none,
                 and we can't have all errors here because we'd need to go beyond the limit. *)
              let to_push =
                FileMap.add to_push ~key:file ~data:PhaseMap.empty
              in
              let error_count_in_ide =
                error_count_in_ide - error_count_before
              in
              let to_push_candidates =
                FileMap.add to_push_candidates ~key:file ~data:phase_map
              in
              (to_push, to_push_candidates, error_count_in_ide))
    in
    (* Keep pushing until we reach the limit. *)
    let (to_push, errors_beyond_limit, _error_count_in_ide) =
      FileMap.fold
        to_push_candidates
        ~init:(to_push, FileMap.empty, error_count_in_ide)
        ~f:(fun file phase_map (to_push, errors_beyond_limit, error_count_in_ide)
           ->
          (* [to_push_candidates] now only contains files which are not in
             [errors_in_ide] or have been deduced from it. *)
          let error_count = error_count_in_phase_map phase_map in
          if error_count_in_ide + error_count <= !error_limit then
            let to_push = FileMap.add to_push ~key:file ~data:phase_map in
            let error_count_in_ide = error_count_in_ide + error_count in
            (to_push, errors_beyond_limit, error_count_in_ide)
          else
            let errors_beyond_limit =
              FileMap.add errors_beyond_limit ~key:file ~data:phase_map
            in
            (to_push, errors_beyond_limit, error_count_in_ide))
    in
    (to_push, errors_beyond_limit)

  (* Convert all paths to absolute paths and union all phases for each file. *)
  let to_diagnostic_errors : ErrorMap.t -> ServerCommandTypes.diagnostic_errors
      =
   fun errors ->
    FileMap.fold errors ~init:SMap.empty ~f:(fun path phase_map errors_acc ->
        let path = Relative_path.to_absolute path in
        let file_errors =
          PhaseMap.fold phase_map ~init:[] ~f:(fun _phase -> ( @ ))
          |> List.map ~f:User_error.to_absolute
        in
        SMap.add ~key:path ~data:file_errors errors_acc)

  let get_errors_to_push :
      t ->
      rechecked:Relative_path.Set.t ->
      new_errors:Errors.t ->
      priority_files:Relative_path.Set.t option ->
      t * ServerCommandTypes.diagnostic_errors =
   fun { errors_in_ide; to_push; errors_beyond_limit }
       ~rechecked
       ~new_errors
       ~priority_files ->
    let phase = PhaseSingleton in
    let new_errors : error list FileMap.t =
      Errors.as_map (Errors.drop_fixmed_errors_in_files new_errors)
    in
    (* Merge to_push and errors_beyond_limit to obtain the total
     * set of errors to be pushed. *)
    let to_push = FileMap.union errors_beyond_limit to_push in
    (* Update that set with the new errors. *)
    let to_push =
      merge_errors_to_push to_push ~rechecked ~errors_in_ide ~new_errors ~phase
    in
    (* Clear any old errors that are no longer there. *)
    let to_push = erase_errors to_push ~errors_in_ide ~rechecked ~phase in
    (* Separate errors we'll push now vs. errors we won't push yet
     * because they are beyond the error limit *)
    let (to_push, errors_beyond_limit) =
      truncate to_push ~errors_in_ide ~priority_files
    in
    let to_push_absolute = to_diagnostic_errors to_push in
    ({ errors_in_ide; to_push; errors_beyond_limit }, to_push_absolute)

  let compute_total_errors : ErrorMap.t -> ErrorMap.t -> ErrorMap.t =
   fun errors_in_ide to_push ->
    FileMap.fold
      to_push
      ~init:errors_in_ide
      ~f:(fun file phase_map errors_in_ide ->
        let phase_map =
          PhaseMap.fold
            phase_map
            ~init:PhaseMap.empty
            ~f:(fun phase errors phase_map ->
              if List.is_empty errors then
                phase_map
              else
                PhaseMap.add phase_map ~key:phase ~data:errors)
        in
        if PhaseMap.is_empty phase_map then
          FileMap.remove errors_in_ide file
        else
          FileMap.add errors_in_ide ~key:file ~data:phase_map)

  let commit_pushed_errors : t -> t =
   fun { errors_in_ide; to_push; errors_beyond_limit } ->
    let all_errors = compute_total_errors errors_in_ide to_push in
    { errors_in_ide = all_errors; to_push = FileMap.empty; errors_beyond_limit }

  let reset_for_new_client : t -> t =
   fun { errors_in_ide; to_push; errors_beyond_limit } ->
    let all_errors = compute_total_errors errors_in_ide to_push in
    { errors_in_ide = FileMap.empty; to_push = all_errors; errors_beyond_limit }

  let get_files_with_diagnostics : t -> Relative_path.t list =
   (fun { errors_in_ide; _ } -> ErrorMap.files errors_in_ide)

  module TestExporter = struct
    let make ~errors_in_ide ~to_push ~errors_beyond_limit =
      { errors_in_ide; to_push; errors_beyond_limit }

    let with_error_limit limit f =
      let previous_error_limit = !error_limit in
      error_limit := limit;
      let result = f () in
      error_limit := previous_error_limit;
      result
  end
end

type t = {
  error_tracker: ErrorTracker.t;
  tracked_ide_id: int option;
}
[@@deriving show]

type push_result =
  | DidPush of { timestamp: seconds_since_epoch }
  | DidNotPush
[@@ocaml.warning "-37"]

let push_result_as_option = function
  | DidPush { timestamp } -> Some timestamp
  | DidNotPush -> None

let init = { error_tracker = ErrorTracker.init; tracked_ide_id = None }

let push_to_client :
    ServerCommandTypes.diagnostic_errors -> ClientProvider.client -> push_result
    =
 (fun _errors _client -> DidNotPush)

(** Reset the error tracker if the new client ID is different from the tracked one. *)
let possibly_reset_tracker { error_tracker; tracked_ide_id } new_ide_id =
  let log_went_away old_id =
    Hh_logger.info
      ~category:"clients"
      "[Diagnostic_pusher] Tracked client with ID %d went away"
      old_id
  in
  let log_new_client new_id =
    Hh_logger.info
      ~category:"clients"
      "[Diagnostic_pusher] New client with ID %d"
      new_id
  in
  let client_went_away =
    match (tracked_ide_id, new_ide_id) with
    | (None, None) -> false
    | (None, Some client_id) ->
      log_new_client client_id;
      false
    | (Some tracked_ide_id, None) ->
      log_went_away tracked_ide_id;
      true
    | (Some tracked_ide_id, Some client_id) ->
      let went_away = not (Int.equal tracked_ide_id client_id) in
      if went_away then (
        log_went_away tracked_ide_id;
        log_new_client client_id
      );
      went_away
  in
  let error_tracker =
    if client_went_away then
      ErrorTracker.reset_for_new_client error_tracker
    else
      error_tracker
  in
  let tracked_ide_id = new_ide_id in
  { error_tracker; tracked_ide_id }

(** Get client from the client provider and possibly reset the error tracker
    if we've lost the previous client. *)
let get_client :
    t -> t * (ClientProvider.client option * Relative_path.Set.t option) =
 (fun pusher -> (possibly_reset_tracker pusher None, (None, None)))

let push_new_errors :
    t ->
    rechecked:Relative_path.Set.t ->
    Errors.t ->
    t * seconds_since_epoch option =
 fun pusher ~rechecked new_errors ->
  let ({ error_tracker; tracked_ide_id }, (client, priority_files)) =
    get_client pusher
  in
  let (error_tracker, to_push) =
    ErrorTracker.get_errors_to_push
      error_tracker
      ~rechecked
      ~new_errors
      ~priority_files
  in
  let push_result =
    match client with
    | None -> DidNotPush
    | Some client -> push_to_client to_push client
  in
  let error_tracker =
    match push_result with
    | DidPush { timestamp = _ } ->
      ErrorTracker.commit_pushed_errors error_tracker
    | DidNotPush -> error_tracker
  in
  ({ error_tracker; tracked_ide_id }, push_result_as_option push_result)

let push_whats_left : t -> t * seconds_since_epoch option =
 fun pusher ->
  push_new_errors ~rechecked:Relative_path.Set.empty pusher Errors.empty

let get_files_with_diagnostics : t -> Relative_path.t list =
 fun { error_tracker; _ } ->
  ErrorTracker.get_files_with_diagnostics error_tracker

module TestExporter = struct
  module FileMap = FileMap
  module ErrorTracker = ErrorTracker
end

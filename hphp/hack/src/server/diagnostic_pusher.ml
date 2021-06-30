(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type error = Errors.error [@@deriving show]

module FileMap = Relative_path.Map

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
    t * Errors.finalized_error list SMap.t

  (** Inform the tracker that errors yet to be pushed have been successfully pushed
      to the IDE. *)
  val commit_pushed_errors : t -> t

  (** Reset the tracker for when the previous client has been lost
      and we have got / will get a new one. *)
  val reset_for_new_client : t -> t

  (** Module to export internal functions for unit testing. Do not use in production code. *)
  module TestExporter : sig
    val make :
      errors_in_ide:error list FileMap.t ->
      to_push:error list FileMap.t ->
      errors_beyond_limit:error list FileMap.t ->
      t

    val with_error_limit : int -> (unit -> 'result) -> 'result
  end
end = struct
  type errors = error list [@@deriving show]

  let equal_errors err1 err2 =
    let err1 = Errors.ErrorSet.of_list err1 in
    let err2 = Errors.ErrorSet.of_list err2 in
    Errors.ErrorSet.equal err1 err2

  type t = {
    errors_in_ide: errors FileMap.t;
        (** The set of errors currently in the IDE, based on what has been pushed so far. *)
    to_push: errors FileMap.t;
        (** The set of errors yet to be pushed to the IDE. *)
    errors_beyond_limit: errors FileMap.t;
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

  let error_count_in_file_map : errors FileMap.t -> int =
    FileMap.fold ~init:0 ~f:(fun _file errors count ->
        count + List.length errors)

  (** For each file key from [rechecked], update value in [errors] with value from
      [new_error], possibly removing entry. *)
  let merge_errors_to_push :
      error list FileMap.t ->
      rechecked:Relative_path.Set.t ->
      new_errors:error list FileMap.t ->
      error list FileMap.t =
   fun errors ~rechecked ~new_errors ->
    let errors =
      Relative_path.Set.fold
        rechecked
        ~init:errors
        ~f:(fun rechecked_file errors -> FileMap.remove errors rechecked_file)
    in
    let errors =
      FileMap.fold new_errors ~init:errors ~f:(fun file file_errors errors ->
          if List.is_empty file_errors then
            (* We'll deal with erasing (which we do by sending the empty list) in erase_errors.
             * This should normally not happen, we're just being defensive here. *)
            errors
          else
            FileMap.add errors ~key:file ~data:file_errors)
    in
    errors

  (** Add [file -> []] entry in the [to_push] map for each file which used to have
      errors but doesn't anymore. *)
  let erase_errors :
      errors_in_ide:error list FileMap.t ->
      rechecked:Relative_path.Set.t ->
      error list FileMap.t ->
      error list FileMap.t =
   fun ~errors_in_ide ~rechecked to_push ->
    let rechecked_which_had_errors =
      Relative_path.Set.filter rechecked ~f:(FileMap.mem errors_in_ide)
    in
    let to_push =
      Relative_path.Set.fold
        rechecked_which_had_errors
        ~init:to_push
        ~f:(fun rechecked_file to_push ->
          if FileMap.mem to_push rechecked_file then
            to_push
          else
            FileMap.add to_push ~key:rechecked_file ~data:[])
    in
    to_push

  let to_absolute_errors errors =
    FileMap.fold errors ~init:SMap.empty ~f:(fun path el acc ->
        SMap.add
          (Relative_path.to_absolute path)
          (List.map el ~f:Errors.to_absolute)
          acc)

  (** Truncate the list of errors to push so that the total number of errors
      in the IDE does not exceed [error_limit]. This partitions the errors as
      a set of errors we can push now and the remainder of the errors. *)
  let truncate to_push_candidates ~errors_in_ide =
    let error_count_in_ide = error_count_in_file_map errors_in_ide in
    (* First push errors in files which already have errors *)
    let (to_push, to_push_candidates, error_count_in_ide) =
      FileMap.fold
        to_push_candidates
        ~init:(FileMap.empty, FileMap.empty, error_count_in_ide)
        ~f:(fun file errors (to_push, to_push_candidates, error_count_in_ide) ->
          let error_count = List.length errors in
          match FileMap.find_opt errors_in_ide file with
          | None ->
            (* Let's consider it again later. *)
            let to_push_candidates =
              FileMap.add to_push_candidates ~key:file ~data:errors
            in
            (to_push, to_push_candidates, error_count_in_ide)
          | Some errors_in_ide ->
            let error_count_before = List.length errors_in_ide in
            if
              error_count_in_ide + error_count - error_count_before
              <= !error_limit
            then
              let error_count_in_ide =
                error_count_in_ide + error_count - error_count_before
              in
              let to_push = FileMap.add to_push ~key:file ~data:errors in
              (to_push, to_push_candidates, error_count_in_ide)
            else
              (* Erase those errors in the IDE, because for each file we want either all errors or none,
                 and we can't have all errors here because we'd need to go beyond the limit. *)
              let to_push = FileMap.add to_push ~key:file ~data:[] in
              let error_count_in_ide =
                error_count_in_ide - error_count_before
              in
              let to_push_candidates =
                FileMap.add to_push_candidates ~key:file ~data:errors
              in
              (to_push, to_push_candidates, error_count_in_ide))
    in
    (* Keep pushing until we reach the limit. *)
    let (to_push, errors_beyond_limit, _error_count_in_ide) =
      FileMap.fold
        to_push_candidates
        ~init:(to_push, FileMap.empty, error_count_in_ide)
        ~f:(fun file
                errors
                (to_push, errors_beyond_limit, error_count_in_ide)
                ->
          (* [to_push_candidates] now only contains files which are not in
             [errors_in_ide] or have been deduced from it. *)
          let error_count = List.length errors in
          if error_count_in_ide + error_count <= !error_limit then
            let to_push = FileMap.add to_push ~key:file ~data:errors in
            let error_count_in_ide = error_count_in_ide + error_count in
            (to_push, errors_beyond_limit, error_count_in_ide)
          else
            let errors_beyond_limit =
              FileMap.add errors_beyond_limit ~key:file ~data:errors
            in
            (to_push, errors_beyond_limit, error_count_in_ide))
    in
    (to_push, errors_beyond_limit)

  let get_errors_to_push :
      t ->
      rechecked:Relative_path.Set.t ->
      new_errors:Errors.t ->
      t * Errors.finalized_error list SMap.t =
   fun { errors_in_ide; to_push; errors_beyond_limit } ~rechecked ~new_errors ->
    let new_errors : error list FileMap.t = Errors.as_map new_errors in
    (* Merge to_push and errors_beyond_limit to obtain the total
     * set of errors to be pushed. *)
    let to_push = FileMap.union errors_beyond_limit to_push in
    (* Update that set with the new errors. *)
    let to_push = merge_errors_to_push to_push ~rechecked ~new_errors in
    (* Clear any old errors that are no longer there. *)
    let to_push = erase_errors to_push ~errors_in_ide ~rechecked in
    (* Separate errors we'll push now vs. errors we won't push yet
     * because they are beyond the error limit *)
    let (to_push, errors_beyond_limit) = truncate to_push ~errors_in_ide in
    let to_push_absolute = to_absolute_errors to_push in
    ({ errors_in_ide; to_push; errors_beyond_limit }, to_push_absolute)

  let compute_total_errors errors_in_ide to_push =
    FileMap.fold
      to_push
      ~init:errors_in_ide
      ~f:(fun file errors errors_in_ide ->
        match errors with
        | [] -> FileMap.remove errors_in_ide file
        | errors -> FileMap.add errors_in_ide ~key:file ~data:errors)

  let commit_pushed_errors : t -> t =
   fun { errors_in_ide; to_push; errors_beyond_limit } ->
    let all_errors = compute_total_errors errors_in_ide to_push in
    { errors_in_ide = all_errors; to_push = FileMap.empty; errors_beyond_limit }

  let reset_for_new_client : t -> t =
   fun { errors_in_ide; to_push; errors_beyond_limit } ->
    let all_errors = compute_total_errors errors_in_ide to_push in
    { errors_in_ide = FileMap.empty; to_push = all_errors; errors_beyond_limit }

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
  tracked_client_id: int option;
}

let init = { error_tracker = ErrorTracker.init; tracked_client_id = None }

let push_to_client :
    Errors.finalized_error list SMap.t -> ClientProvider.client -> bool =
 fun errors client ->
  if ClientProvider.client_has_message client then
    false
  else if SMap.is_empty errors then
    false
  else
    let res = ServerCommandTypes.DIAGNOSTIC { errors; is_truncated = None } in
    try
      ClientProvider.send_push_message_to_client client res;
      true
    with ClientProvider.Client_went_away -> false

(** Reset the error tracker if the new client ID is different from the tracked one. *)
let possibly_reset_tracker { error_tracker; tracked_client_id } new_client_id =
  let client_went_away =
    match (tracked_client_id, new_client_id) with
    | (None, _) -> false
    | (Some _, None) -> true
    | (Some tracked_client_id, Some client_id) ->
      not (Int.equal tracked_client_id client_id)
  in
  let error_tracker =
    if client_went_away then
      ErrorTracker.reset_for_new_client error_tracker
    else
      error_tracker
  in
  let tracked_client_id = new_client_id in
  { error_tracker; tracked_client_id }

let option_of_tuple_to_tuple_of_options :
    ('a * 'b) option -> 'a option * 'b option = function
  | None -> (None, None)
  | Some (a, b) -> (Some a, Some b)

(** Get client from the client provider and possibly reset the error tracker
    if we've lost the previous client. *)
let get_client : t -> t * ClientProvider.client option =
 fun pusher ->
  let (current_client_id, current_client) =
    ClientProvider.get_persistent_client ()
    |> option_of_tuple_to_tuple_of_options
  in
  let pusher = possibly_reset_tracker pusher current_client_id in
  (pusher, current_client)

let push_new_errors : t -> rechecked:Relative_path.Set.t -> Errors.t -> t =
 fun pusher ~rechecked new_errors ->
  let ({ error_tracker; tracked_client_id }, client) = get_client pusher in
  let (error_tracker, to_push) =
    ErrorTracker.get_errors_to_push error_tracker ~rechecked ~new_errors
  in
  let was_pushed =
    match client with
    | None -> false
    | Some client -> push_to_client to_push client
  in
  let error_tracker =
    if was_pushed then
      ErrorTracker.commit_pushed_errors error_tracker
    else
      error_tracker
  in
  { error_tracker; tracked_client_id }

module TestExporter = struct
  module FileMap = FileMap
  module ErrorTracker = ErrorTracker
end

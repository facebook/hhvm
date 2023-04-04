(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module RemoteLogging = Sdt_analysis_remote_logging

module Reverts = struct
  type revert = {
    file: string;
    contents: string;
  }

  type t = revert list

  let of_patches patches : t =
    let open List.Monad_infix in
    patches
    >>| ServerRefactorTypes.get_pos
    >>| Pos.filename
    |> List.dedup_and_sort ~compare:compare_string
    >>| fun file -> { file; contents = Disk.cat file }

  let apply : t -> unit =
    List.iter ~f:(fun { file; contents } -> Disk.write_file ~file ~contents)
end

let apply_all
    ~get_error_count
    ~get_patches
    ~apply_patches
    ~path_to_jsonl
    ~strategy
    ~log_remotely
    ~tag =
  let remote_logging = RemoteLogging.create ~strategy ~log_remotely ~tag in
  let%lwt (baseline_error_count, init_telemetry) = get_error_count () in

  let handle_codemod_group codemod_line ~line_index =
    let%lwt ((patches, patched_ids, target_kind), telemetry) =
      get_patches codemod_line
    in
    if List.is_empty patches then
      Lwt.return telemetry
    else begin
      let reverts = Reverts.of_patches patches in
      apply_patches patches;
      let%lwt (error_count, more_telemetry) = get_error_count () in
      let telemetry = Telemetry.add telemetry more_telemetry in
      let error_count_diff = error_count - baseline_error_count in
      assert (error_count_diff > -1);
      let should_revert =
        match strategy with
        | `CodemodSdtCumulative -> error_count_diff > 0
        | `CodemodSdtIndependent -> true
      in
      RemoteLogging.submit_patch_result
        remote_logging
        ~patched_ids
        ~target_kind
        ~error_count:error_count_diff
        ~line_index;
      if should_revert then (
        Reverts.apply reverts;
        let%lwt (error_count, more_telemetry) = get_error_count () in
        assert (error_count = baseline_error_count);
        Lwt.return @@ Telemetry.add telemetry more_telemetry
      ) else
        Lwt.return telemetry
    end
  in

  let combine_line_results acc line =
    let%lwt (telem_acc, line_index) = acc in
    let%lwt telem = handle_codemod_group line ~line_index in
    Lwt.return (Telemetry.add telem_acc telem, line_index + 1)
  in

  let%lwt (telemetry, _) =
    let init = Lwt.return (init_telemetry, 0) in
    In_channel.with_file
      path_to_jsonl
      ~f:(In_channel.fold_lines ~init ~f:combine_line_results)
  in
  Lwt.return (Exit_status.No_error, telemetry)

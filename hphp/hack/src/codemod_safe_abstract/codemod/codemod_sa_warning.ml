(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  warning_code: Error_codes.Warning.t;
  pos: Pos.t;
}

(* Must be > 0 *)
let source_text_cache_capacity = 1000

(* calculate or get from least-recently-inserted cache *)
let get_source_text : Relative_path.t -> Full_fidelity_source_text.t =
  let cache = ref Relative_path.Map.empty in
  (* keep track insertion order, which Relative_path.Map does not maintain *)
  let key_queue = Queue.create ~capacity:source_text_cache_capacity () in
  let add_to_cache key data =
    if Queue.length key_queue >= source_text_cache_capacity then begin
      (* peek_exn and deque_exn are safe because we know length > 0 *)
      cache := Relative_path.Map.remove !cache (Queue.peek_exn key_queue);
      ignore @@ Queue.dequeue_exn key_queue
    end;
    Queue.enqueue key_queue key;
    cache := Relative_path.Map.add !cache ~key ~data
  in
  fun path : Full_fidelity_source_text.t ->
    match Relative_path.Map.find_opt !cache path with
    | Some source_text -> source_text
    | None ->
      let source_text =
        Full_fidelity_source_text.make
          path
          (Disk.cat (Relative_path.to_absolute path))
      in
      let () = add_to_cache path source_text in
      source_text

(** parse the format returned from `hh --json` into our warning representation *)
let parse_warnings_json_exn (warnings_json : Yojson.Safe.t) ~error_message :
    t list Relative_path.Map.t =
  let extract_key_exn key = function
    | `Assoc assoc ->
      List.Assoc.find assoc ~equal:String.equal key
      |> Option.value_exn ~message:error_message
    | _ -> failwith error_message
  in
  let extract_int_exn = function
    | `Int i -> i
    | _ -> failwith error_message
  in
  let extract_string_exn = function
    | `String s -> s
    | _ -> failwith error_message
  in
  let extract_list_exn = function
    | `List l -> l
    | _ -> failwith error_message
  in
  warnings_json
  |> extract_key_exn "errors"
  |> extract_list_exn
  |> List.filter_map ~f:(fun (warning_json : Yojson.Safe.t) : t option ->
         let open Option.Let_syntax in
         let messages =
           extract_key_exn "message" warning_json |> extract_list_exn
         in
         let first_message =
           messages |> List.hd |> Option.value_exn ~message:error_message
         in
         let raw_error_code =
           extract_key_exn "code" first_message |> extract_int_exn
         in
         let descr =
           extract_key_exn "descr" first_message |> extract_string_exn
         in
         (* TODO better handling of unexpected cases *)
         let* warning_code = Error_codes.Warning.of_enum raw_error_code in
         (* TODO better handling of unexpected cases *)
         let+ message =
           match warning_code with
           | Error_codes.Warning.CallNeedsConcrete
             when not (String.is_substring descr ~substring:"via") ->
             (* This means the call is via class ID. The remediation is *not* to
              * add __NeedsConcrete in such cases. Instead, there is no straightforward
              * code change and intent must be discerned and non-file-local changes are likely needed.
              * For other calls we say "via `parent`", "via `self`", or "via `static`"
              *)
             None
           | Error_codes.Warning.CallNeedsConcrete
           | Error_codes.Warning.AbstractAccessViaStatic
           | Error_codes.Warning.UninstantiableClassViaStatic ->
             Some first_message
           | _ -> None
         in
         let pos =
           let path =
             extract_key_exn "path" message
             |> extract_string_exn
             |> Relative_path.create Relative_path.Root
           in
           let line = extract_key_exn "line" message |> extract_int_exn in
           (* TODO: revisit or explain *)
           let line = line in
           let beginning_of_line =
             let source_text = get_source_text path in
             Full_fidelity_source_text.position_to_offset source_text (line, 0)
           in
           let start = extract_key_exn "start" message |> extract_int_exn in
           let end_ = extract_key_exn "end" message |> extract_int_exn in
           Pos.make_from_lnum_bol_offset
             ~pos_file:path
             ~pos_start:(line, beginning_of_line, beginning_of_line + start)
             ~pos_end:(line, beginning_of_line, beginning_of_line + end_)
         in
         { pos; warning_code })
  |> List.fold ~init:Relative_path.Map.empty ~f:(fun acc (warning : t) ->
         let path = Pos.filename warning.pos in
         Relative_path.Map.update
           path
           (fun warnings ->
             let warnings = Option.value warnings ~default:[] in
             Some (warning :: warnings))
           acc)

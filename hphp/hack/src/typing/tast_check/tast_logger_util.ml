(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Generated_file_utils : sig
  val is_generated : Pos.t -> bool
end = struct
  (** A manual (non-generated) section within a partially-generated file.
    [start_offset] is inclusive, [end_offset] is exclusive.
    This matches [Pos.t] conventions (see pos.mli). *)
  type manual_section = {
    start_offset: int;  (** 0-indexed, inclusive *)
    end_offset: int;  (** 0-indexed, exclusive *)
  }

  (** Classification of a file's generated status. *)
  type file_generated_kind =
    | Not_generated
    | Fully_generated
    | Partially_generated of manual_section list
        (** The manual sections are the non-generated regions of the file. *)

  let begin_manual_section_prefix = "/* BEGIN MANUAL SECTION"

  let end_manual_section_tag = "/* END MANUAL SECTION */"

  (** Find manual sections in partially-generated files *)
  let parse_manual_sections (contents : string) : manual_section list =
    let len = String.length contents in
    let rec find_sections acc search_from =
      match
        String.substr_index
          contents
          ~pos:search_from
          ~pattern:begin_manual_section_prefix
      with
      | None -> List.rev acc
      | Some begin_start ->
        (* Find the closing */ of the BEGIN comment.
           Start searching 2 bytes in to skip past the opening "/*". *)
        (match
           String.substr_index contents ~pos:(begin_start + 2) ~pattern:"*/"
         with
        | None -> List.rev acc (* malformed, stop *)
        | Some close_star ->
          let start_offset = close_star + 2 in
          (* Find the matching END MANUAL SECTION *)
          (match
             String.substr_index
               contents
               ~pos:start_offset
               ~pattern:end_manual_section_tag
           with
          | None ->
            (* Unclosed section, skip and keep searching *)
            find_sections acc (min len start_offset)
          | Some end_offset ->
            let section = { start_offset; end_offset } in
            let after_end = end_offset + String.length end_manual_section_tag in
            find_sections (section :: acc) (min len after_end)))
    in
    find_sections [] 0

  let classify_contents (contents : string) : file_generated_kind =
    let partially_generated_tag = "@" ^ "partially-generated" in
    if String.is_substring contents ~substring:partially_generated_tag then
      Partially_generated (parse_manual_sections contents)
    else
      let generated_tag = "@" ^ "generated" in
      if String.is_substring contents ~substring:generated_tag then
        Fully_generated
      else
        Not_generated

  let is_within_manual_section
      (manual_sections : manual_section list) (pos : Pos.t) : bool =
    let pos_start = Pos.start_offset pos in
    let pos_end = Pos.end_offset pos in
    List.exists manual_sections ~f:(fun section ->
        pos_start >= section.start_offset && pos_end <= section.end_offset)

  (** A single-entry cache, since each process checks one file at a time *)
  let cache : (Relative_path.t * file_generated_kind) option ref = ref None

  let classify_file (file : Relative_path.t) : file_generated_kind option =
    match !cache with
    | Some (cached_file, kind) when Relative_path.equal cached_file file ->
      Some kind
    | _ ->
      (match File_provider.get_contents file with
      | None -> None
      | Some contents ->
        let kind = classify_contents contents in
        cache := Some (file, kind);
        Some kind)

  let is_generated (pos : Pos.t) : bool =
    let file = Pos.filename pos in
    match classify_file file with
    | None -> false
    | Some Not_generated -> false
    | Some Fully_generated -> true
    | Some (Partially_generated manual_sections) ->
      not (is_within_manual_section manual_sections pos)
end

let is_generated = Generated_file_utils.is_generated

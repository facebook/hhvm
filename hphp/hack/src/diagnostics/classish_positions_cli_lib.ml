(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let apply_patches_to_string
    old_content (patches : Server_rename_types.patch list) : string =
  let buf = Buffer.create (String.length old_content) in
  let patch_list =
    List.sort ~compare:Server_rename_types.compare_result patches
  in
  Server_rename_types.write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

let pos_to_patches (pos : Pos.t) : Server_rename_types.patch list =
  let pos = Pos.to_absolute pos in
  Server_rename_types.
    [
      Replace { pos = Pos.shrink_to_start pos; text = ">" };
      Replace { pos = Pos.shrink_to_end pos; text = "<" };
    ]

let classish_positions_for_class_to_patches
    (cp : Pos.t Classish_positions_types.classish_positions) :
    Server_rename_types.patch list S_map.t =
  let Classish_positions_types.
        {
          classish_start_of_body;
          classish_end_of_body;
          classish_closing_brace;
          classish_body_elements;
        } =
    cp
  in
  S_map.of_list
    [
      ("classish_start_of_body", pos_to_patches classish_start_of_body);
      ("classish_end_of_body", pos_to_patches classish_end_of_body);
      ("classish_closing_brace", pos_to_patches classish_closing_brace);
      ( "classish_body_elements",
        List.bind ~f:pos_to_patches classish_body_elements );
    ]

let classish_positions_to_patches
    (cps : Pos.t Classish_positions_types.classish_positions S_map.t) :
    Server_rename_types.patch list S_map.t =
  let merge_map_entry _key xs ys =
    match (xs, ys) with
    | (None, None) -> None
    | (Some xs, None) -> Some xs
    | (None, Some ys) -> Some ys
    | (Some xs, Some ys) -> Some (xs @ ys)
  in
  List.fold (S_map.values cps) ~init:S_map.empty ~f:(fun acc cp ->
      S_map.merge
        merge_map_entry
        acc
        (classish_positions_for_class_to_patches cp))

let apply_patch_for source_text positional_category patches =
  Printf.printf "%s\n" positional_category;
  Printf.printf "%s\n" (String.make (String.length positional_category) '-');
  Printf.printf "%s\n\n" (apply_patches_to_string source_text patches)

let run_exn ctx entry path =
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in

  let classish_positions =
    match entry.Provider_context.source_text with
    | Some source_text ->
      Classish_positions.extract tree source_text entry.Provider_context.path
    | None -> Classish_positions.empty
  in
  let named_patches = classish_positions_to_patches classish_positions in

  let source_text = Sys_utils.cat @@ Relative_path.to_absolute path in
  S_map.iter (apply_patch_for source_text) named_patches

let dump ctx entry path =
  match run_exn ctx entry path with
  | exception exn -> print_endline @@ Exn.to_string exn
  | () -> ()

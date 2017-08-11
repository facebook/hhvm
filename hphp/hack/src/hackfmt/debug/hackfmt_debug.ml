(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxTree = Full_fidelity_syntax_tree
module SourceText = Full_fidelity_source_text

open Core

type debug_config = {
  print_ast: bool;
  print_doc: bool;
  print_nesting_graph: bool;
  print_rule_dependencies: bool;
  chunk_ids: int list option;
}

let debug_config = ref {
  print_ast = false;
  print_doc = false;
  print_nesting_graph = false;
  print_rule_dependencies = false;
  chunk_ids = None;
}

let init_with_options () = [
  "--ast",
  Arg.Unit (fun () -> debug_config := { !debug_config with print_ast = true }),
  " Print out an ast dump before the formatted result ";
  "--doc",
  Arg.Unit (fun () ->
    debug_config := { !debug_config with print_doc = true }),
  " Print out a dump of the Doc IR";
  "--ids",
  Arg.String (fun s ->
    debug_config := { !debug_config with chunk_ids = Some (
      try List.map (Str.split (Str.regexp ",") s) ~f:int_of_string
      with Failure _ -> raise (Failure "Invalid id list specification")
    )}
  ),
  " Comma separate list of chunk ids to inspect (default is all)";
  "--nesting",
  Arg.Unit (fun () ->
    debug_config := { !debug_config with print_nesting_graph = true }),
  " Print out a representation of the nesting graph below each chunk group";
  "--rule-deps",
  Arg.Unit (fun () ->
    debug_config := { !debug_config with print_rule_dependencies = true }),
  " Print out a representation of the rule dependenies below each chunk group";
]

let debug_nesting chunk_group =
  List.iteri chunk_group.Chunk_group.chunks ~f:(fun i c ->
    let nesting_list =
      Nesting.get_self_and_parent_list (Some c.Chunk.nesting) in
    let list_string = String.concat ", "
      @@ List.map nesting_list ~f:string_of_int in
    Printf.printf "Chunk %d - [ %s ]\n" i list_string;
  )

let debug_rule_deps chunk_group =
  Printf.printf "%s\n" (Chunk_group.dependency_map_to_string chunk_group)

let debug_chunk_groups env ~range source_text chunk_groups =
  let print_chunk = match !debug_config.chunk_ids with
    | None -> (fun id c -> Some (id, c))
    | Some id_list -> (fun id c ->
        if List.exists id_list (fun x -> x = id) then Some (id, c) else None
      )
  in
  chunk_groups
  |> List.filter_mapi ~f:print_chunk
  |> List.filter ~f:(fun (i, cg) ->
    let group_range = Chunk_group.get_char_range cg in
    Interval.intervals_overlap range group_range
  )
  |> List.iter ~f:(fun (i, cg) ->
    Printf.printf "Group Id: %d\n" i;
    Printf.printf "Indentation: %d\n" cg.Chunk_group.block_indentation;
    Printf.printf "Chunk count: %d\n" (List.length cg.Chunk_group.chunks);
    List.iteri cg.Chunk_group.chunks ~f:(fun i c ->
      Printf.printf "%8d rule_id:%d [%d,%d)\t%s\n"
        i
        c.Chunk.rule
        c.Chunk.start_char
        c.Chunk.end_char
        c.Chunk.text
    );
    Printf.printf "Rule count %d\n"
      (IMap.cardinal cg.Chunk_group.rule_map);
    IMap.iter (fun k v ->
      Printf.printf "%8d - %s\n" k (Rule.to_string v);
    ) cg.Chunk_group.rule_map;

    if !debug_config.print_rule_dependencies then debug_rule_deps cg;
    if !debug_config.print_nesting_graph then debug_nesting cg;

    Printf.printf "%s\n"
      (Line_splitter.solve env ~range (SourceText.text source_text) [cg]);
  )

let debug_full_text source_text =
  Printf.printf "%s\n" (SourceText.get_text source_text)

let debug_ast syntax_tree =
  Printf.printf "%s\n" @@ Debug.dump_full_fidelity syntax_tree

let debug_text_range source_text start_char end_char =
  Printf.printf "Subrange passed:\n%s\n" @@
    String.sub source_text.SourceText.text start_char (end_char - start_char)

let debug env ~range source_text syntax_tree doc chunk_groups =
  if !debug_config.print_ast then debug_ast syntax_tree;
  if !debug_config.print_doc then ignore (Doc.dump doc);
  let range = Option.value range
    ~default:(0, Full_fidelity_source_text.length source_text)
  in
  debug_chunk_groups env ~range source_text chunk_groups

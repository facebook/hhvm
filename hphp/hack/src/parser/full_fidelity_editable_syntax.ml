(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Editable syntax tree
 *
 *  Every token and trivia in the tree knows its text. Therefore we can add
 *  new nodes without having to back them with a source text.
 *)

module Token = Full_fidelity_editable_token
module Trivia = Full_fidelity_editable_trivia
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SyntaxWithToken = Full_fidelity_syntax.WithToken (Token)

(**
 * Ironically, an editable syntax tree needs even less per-node information
 * than the "positioned" syntax tree, which needs to know the width of the node.
 **)

module Value = struct
  type t = NoValue [@@deriving show, eq]

  let to_json _value = Hh_json.(JSON_Object [])
end

module EditableSyntax = SyntaxWithToken.WithSyntaxValue (Value)

module EditableValueBuilder = struct
  let value_from_children _ _ _ _ = Value.NoValue

  let value_from_token _ = Value.NoValue

  let value_from_syntax _ = Value.NoValue
end

include EditableSyntax
include EditableSyntax.WithValueBuilder (EditableValueBuilder)

let text node =
  let buffer = Buffer.create 100 in
  let aux token = Buffer.add_string buffer (Token.full_text token) in
  List.iter aux (all_tokens node);
  Buffer.contents buffer

let extract_text node = Some (text node)

let width node = String.length (text node)

(* Takes a node and an offset; produces the descent through the parse tree
   to that position. *)
let parentage root position =
  let rec aux nodes position acc =
    match nodes with
    | [] -> acc
    | h :: t ->
      let width = String.length @@ text h in
      if position < width then
        aux (children h) position (h :: acc)
      else
        aux t (position - width) acc
  in
  aux [root] position []

let leading_trivia node =
  let token = leading_token node in
  match token with
  | None -> []
  | Some t -> Token.leading t

let leading_width node =
  leading_trivia node |> List.fold_left (fun sum t -> sum + Trivia.width t) 0

let trailing_trivia node =
  let token = trailing_token node in
  match token with
  | None -> []
  | Some t -> Token.trailing t

let trailing_width node =
  trailing_trivia node |> List.fold_left (fun sum t -> sum + Trivia.width t) 0

let full_width node = leading_width node + width node + trailing_width node

let is_in_body node position =
  let rec aux parents =
    match parents with
    | [] -> false
    | h1 :: h2 :: _
      when is_compound_statement h1
           && (is_methodish_declaration h2 || is_function_declaration h2) ->
      true
    | _ :: rest -> aux rest
  in
  let parents = parentage node position in
  aux parents

(* This function takes a parse tree and renders it in the GraphViz DOT
language; this is a small domain-specific language for visualizing graphs.
You can use www.webgraphviz.com to render it in a browser, or the "dot"
command line tool to turn DOT text into image files.

Edge labels can make the graph hard to read, so they can be enabled or
disabled as you like.

Use hh_parse --full-fidelity-dot or --full-fidelity-dot-edges to parse
a Hack file and display it in DOT form.

TODO: There's nothing here that's unique to editable trees; this could
be auto-generated as part of full_fidelity_syntax.ml.
*)
let to_dot node with_labels =
  (* returns new current_id, accumulator *)
  let rec aux node current_id parent_id edge_label acc =
    let kind = SyntaxKind.to_string (kind node) in
    let new_id = current_id + 1 in
    let label =
      if with_labels then
        Printf.sprintf " [label=\"%s\"]" edge_label
      else
        ""
    in
    let new_edge = Printf.sprintf "  %d -> %d%s" parent_id current_id label in
    match node.syntax with
    | Token t ->
      (* TODO: Trivia *)
      let kind = TokenKind.to_string (Token.kind t) in
      let new_node = Printf.sprintf "  %d [label=\"%s\"];" current_id kind in
      let new_acc = new_edge :: new_node :: acc in
      (new_id, new_acc)
    | SyntaxList x ->
      let new_node = Printf.sprintf "  %d [label=\"%s\"];" current_id kind in
      let new_acc = new_edge :: new_node :: acc in
      let folder (c, a) n = aux n c current_id "" a in
      List.fold_left folder (new_id, new_acc) x
    | _ ->
      let folder (c, a) n l = aux n c current_id l a in
      let new_node = Printf.sprintf "  %d [label=\"%s\"];" current_id kind in
      let new_acc = new_edge :: new_node :: acc in
      List.fold_left2
        folder
        (new_id, new_acc)
        (children node)
        (children_names node)
  in
  let (_, acc) =
    aux node 1001 1000 "" ["  1000 [label=\"root\"]"; "digraph {"]
  in
  let acc = "}" :: acc in
  String.concat "\n" (List.rev acc)

let offset _ = None

let position _ _ = None

let to_json ?with_value:_ ?ignore_missing:_ node =
  let version = Full_fidelity_schema.full_fidelity_schema_version_number in
  let tree = EditableSyntax.to_json node in
  Hh_json.JSON_Object
    [("parse_tree", tree); ("version", Hh_json.JSON_String version)]

let rust_parse _ _ = failwith "not implemented"

let rust_parse_with_verify_sc _ _ = failwith "not implemented"

let rust_parser_errors _ _ _ = failwith "not implemented"

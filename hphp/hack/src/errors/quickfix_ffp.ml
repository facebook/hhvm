(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Full_fidelity_positioned_syntax

let token_name (s : Syntax.t) : string option =
  let open Syntax in
  match s.syntax with
  | Token t ->
    let name = t.Token.source_text.Token.SourceText.text in
    Some name
  | _ -> None

let classish_body_brace_offset s : int option =
  let open Syntax in
  match s.syntax with
  | ClassishBody cb ->
    let open_brace = cb.classish_body_left_brace in
    (match open_brace.syntax with
    | Token t -> Some (t.Token.offset + t.Token.width)
    | _ -> None)
  | _ -> None

let classish_start_offsets (s : Syntax.t) : int SMap.t =
  let open Syntax in
  let rec aux (acc : int SMap.t) (s : Syntax.t) =
    match s.syntax with
    | Syntax.Script s -> aux acc s.script_declarations
    | Syntax.SyntaxList sl -> List.fold sl ~init:acc ~f:aux
    | Syntax.NamespaceDeclaration n -> aux acc n.namespace_body
    | Syntax.ClassishDeclaration c ->
      (match
         (token_name c.classish_name, classish_body_brace_offset c.classish_body)
       with
      | (Some name, Some offset) -> SMap.add name offset acc
      | _ -> acc)
    | _ -> acc
  in

  aux SMap.empty s

(** Return the position of the start "{" in every classish in this
    file. *)
let classish_starts
    (s : Syntax.t)
    (source_text : Full_fidelity_source_text.t)
    (filename : Relative_path.t) : Pos.t SMap.t =
  let offsets = classish_start_offsets s in
  let to_pos offset =
    Full_fidelity_source_text.relative_pos filename source_text offset offset
  in
  SMap.map to_pos offsets

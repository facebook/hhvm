(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Full_fidelity_positioned_syntax

let classish_body_brace_offset s : int option =
  let open Syntax in
  match s.syntax with
  | ClassishBody cb ->
    let open_brace = cb.classish_body_left_brace in
    (match open_brace.syntax with
    | Token t -> Some (t.Token.offset + t.Token.width)
    | _ -> None)
  | _ -> None

let namespace_name (s : Syntax.t) : string option =
  let open Syntax in
  match s.syntax with
  | Syntax.NamespaceDeclarationHeader h ->
    let name_token = h.namespace_name in
    (match name_token.syntax with
    | Syntax.Token _ -> Some (text name_token)
    | _ ->
      (* Anonymous namespace: namespace { ... } *)
      None)
  | _ -> None

(* Covnert ["Foo"; "Bar"] to \Foo\Bar. *)
let name_from_parts (parts : string list) : string =
  String.concat (List.map parts ~f:(fun p -> "\\" ^ p))

let classish_start_offsets (s : Syntax.t) : int SMap.t =
  let open Syntax in
  let rec aux (acc : int SMap.t * string list) (s : Syntax.t) =
    let (offsets, namespace) = acc in
    match s.syntax with
    | Syntax.Script s -> aux acc s.script_declarations
    | Syntax.SyntaxList sl -> List.fold sl ~init:acc ~f:aux
    | Syntax.NamespaceDeclaration n ->
      let b = n.namespace_body in
      (match b.syntax with
      | Syntax.NamespaceBody nb ->
        (* We're looking at: namespace Foo { ... } *)
        let inner_namespace =
          match namespace_name n.namespace_header with
          | Some name -> name :: namespace
          | None -> namespace
        in
        let (offsets, _) =
          aux (offsets, inner_namespace) nb.namespace_declarations
        in
        (offsets, namespace)
      | Syntax.NamespaceEmptyBody _ ->
        (* We're looking at: namespace Foo; *)
        let namespace =
          match namespace_name n.namespace_header with
          | Some name -> name :: namespace
          | None -> namespace
        in
        (offsets, namespace)
      | _ -> acc)
    | Syntax.ClassishDeclaration c ->
      (match classish_body_brace_offset c.classish_body with
      | Some offset ->
        let name = name_from_parts (namespace @ [text c.classish_name]) in
        let offsets = SMap.add name offset offsets in
        (offsets, namespace)
      | _ -> acc)
    | _ -> acc
  in

  fst (aux (SMap.empty, []) s)

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

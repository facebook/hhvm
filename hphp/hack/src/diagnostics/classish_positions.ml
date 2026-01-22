(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Full_fidelity_positioned_syntax
include Classish_positions_types

let classish_body_elements_positions_for ~(to_pos : int -> int -> Pos.t) s :
    Pos.t list =
  let open Syntax in
  let range_for_body_element e =
    let open Option.Let_syntax in
    let* leading_token = leading_token e in
    let* trailing_token = trailing_token e in
    let offset_start =
      leading_token.Token.offset + leading_token.Token.leading_width
    in
    let offset_end =
      trailing_token.Token.offset
      + trailing_token.Token.leading_width
      + trailing_token.Token.width
    in
    Some (to_pos offset_start offset_end)
  in
  match s.syntax with
  | SyntaxList elements -> List.filter_map elements ~f:range_for_body_element
  | _ -> []

let classish_positions_for ~(to_pos : int -> int -> Pos.t) s :
    Pos.t classish_positions option =
  let open Syntax in
  let map_brace_token brace (f : Token.t -> 'a) : 'a option =
    match brace.syntax with
    | Token t -> Some (f t)
    | _ -> None
  in
  let brace_to_offset brace (f : offset:int -> width:int -> int) : int option =
    map_brace_token brace (fun t ->
        f ~offset:t.Token.offset ~width:t.Token.width)
  in
  match s.syntax with
  | ClassishBody cb ->
    let open Option.Let_syntax in
    let* classish_start_of_body_offset =
      brace_to_offset cb.classish_body_left_brace @@ fun ~offset ~width ->
      offset + width
    in
    let* classish_end_of_body_offset =
      brace_to_offset cb.classish_body_right_brace @@ fun ~offset ~width:_ ->
      offset
    in
    let* classish_just_before_closing_brace_offset =
      map_brace_token cb.classish_body_right_brace @@ fun t ->
      t.Token.offset + t.Token.leading_width
    in
    let* classish_just_after_closing_brace_offset =
      map_brace_token cb.classish_body_right_brace @@ fun t ->
      t.Token.offset + t.Token.leading_width + t.Token.width
    in
    let classish_body_elements =
      classish_body_elements_positions_for ~to_pos cb.classish_body_elements
    in
    Some
      {
        classish_start_of_body =
          to_pos classish_start_of_body_offset classish_start_of_body_offset;
        classish_end_of_body =
          to_pos classish_end_of_body_offset classish_end_of_body_offset;
        classish_closing_brace =
          to_pos
            classish_just_before_closing_brace_offset
            classish_just_after_closing_brace_offset;
        classish_body_elements;
      }
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

let classish_positions_of_syntax ~(to_pos : int -> int -> Pos.t) (s : Syntax.t)
    : Pos.t t =
  let rec aux (acc : Pos.t t * string list) (s : Syntax.t) =
    let open Syntax in
    let (positions, namespace) = acc in
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
        let (positions, _) =
          aux (positions, inner_namespace) nb.namespace_declarations
        in
        (positions, namespace)
      | Syntax.NamespaceEmptyBody _ ->
        (* We're looking at: namespace Foo; *)
        let namespace =
          match namespace_name n.namespace_header with
          | Some name -> name :: namespace
          | None -> namespace
        in
        (positions, namespace)
      | _ -> acc)
    | Syntax.ClassishDeclaration c ->
      (match classish_positions_for ~to_pos c.classish_body with
      | Some classish_positions ->
        let name = name_from_parts (namespace @ [text c.classish_name]) in
        let positions = SMap.add name classish_positions positions in
        (positions, namespace)
      | _ -> acc)
    | _ -> acc
  in

  fst (aux (SMap.empty, []) s)

let empty = SMap.empty

let map_classish_positions ~f (classish_positions : 'pos classish_positions) =
  let {
    classish_start_of_body;
    classish_end_of_body;
    classish_closing_brace;
    classish_body_elements;
  } =
    classish_positions
  in
  {
    classish_start_of_body = f classish_start_of_body;
    classish_end_of_body = f classish_end_of_body;
    classish_closing_brace = f classish_closing_brace;
    classish_body_elements = List.map ~f classish_body_elements;
  }

let map ~f (t : _ t) = SMap.map (map_classish_positions ~f) t

let extract
    (s : Syntax.t)
    (source_text : Full_fidelity_source_text.t)
    (filename : Relative_path.t) : Pos.t t =
  let to_pos offset_start offset_end =
    Full_fidelity_source_text.relative_pos
      filename
      source_text
      offset_start
      offset_end
  in
  classish_positions_of_syntax ~to_pos s

let map_pos ~f = function
  | Precomputed p -> Precomputed (f p)
  | ( Classish_start_of_body _ | Classish_end_of_body _
    | Classish_closing_brace _ ) as p ->
    p

let find pos t =
  let map_class class_name f = SMap.find_opt class_name t |> Option.map ~f in
  match pos with
  | Precomputed pos -> Some pos
  | Classish_start_of_body class_name ->
    map_class class_name @@ fun c -> c.classish_start_of_body
  | Classish_end_of_body class_name ->
    map_class class_name @@ fun c -> c.classish_end_of_body
  | Classish_closing_brace class_name ->
    map_class class_name @@ fun c -> c.classish_closing_brace

let inbetween_body_element_positions t : Pos.t list SMap.t =
  let inbetween_positions_for (cp : 'a Pos.pos classish_positions) :
      'a Pos.pos list =
    let classish_body_elements =
      List.sort
        ~compare:(Pos.compare_pos (fun _ _ -> 0))
        cp.classish_body_elements
    in
    let (current_pos, posl) =
      List.fold_left
        classish_body_elements
        ~init:(cp.classish_start_of_body, [])
        ~f:(fun (current_pos, acc) next_pos ->
          let whitespace =
            Pos.btw
              (Pos.shrink_to_end current_pos)
              (Pos.shrink_to_start next_pos)
          in
          (next_pos, whitespace :: acc))
    in
    List.rev
      (Pos.btw
         (Pos.shrink_to_end current_pos)
         (Pos.shrink_to_start cp.classish_end_of_body)
      :: posl)
  in
  SMap.map inbetween_positions_for t

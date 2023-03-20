(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sdt_analysis_types
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module Syn = Full_fidelity_editable_positioned_syntax
module Refactor = Full_fidelity_refactor

type 'a direction =
  | Continue of 'a
  | Stop of 'a

let rec fold_syntax ~f ~init node =
  match f init node with
  | Continue acc ->
    Syn.children node
    |> List.fold ~init:acc ~f:(fun acc node -> fold_syntax ~f ~init:acc node)
  | Stop acc -> acc

let sid_equal s1 s2 = String.equal (Utils.add_ns s1) (Utils.add_ns s2)

let get_function_name_exn node =
  let fold name_opt node =
    match Syn.syntax node with
    | Syn.FunctionDeclarationHeader { function_name; _ } ->
      Stop (Some (Syn.text function_name))
    | _ -> Continue name_opt
  in
  fold_syntax ~f:fold ~init:None node |> Option.value_exn

let patches_of_nadable_id ~source ~path id =
  let root =
    let source_text = Full_fidelity_source_text.make path source in
    let positioned_tree = PositionedTree.make source_text in
    Full_fidelity_editable_positioned_syntax.from_positioned_syntax
      (PositionedTree.root positioned_tree)
  in
  let collect_patches patches node =
    let nad_patch attributes_node =
      Refactor.insert_attribute
        path
        ~attribute:Naming_special_names.UserAttributes.uaNoAutoDynamic
        ~enclosing_node:(Some node)
        ~attributes_node
      |> Option.to_list
    in
    match (id, Syn.syntax node) with
    | ( H.Id.Function sid,
        Syn.FunctionDeclaration
          { function_declaration_header; function_attribute_spec; _ } )
      when sid_equal sid (get_function_name_exn function_declaration_header) ->
      Stop (nad_patch function_attribute_spec @ patches)
    | ( H.Id.ClassLike sid,
        Syn.ClassishDeclaration { classish_name; classish_attribute; _ } )
      when sid_equal sid (Syn.text classish_name) ->
      Stop (nad_patch classish_attribute @ patches)
    | (_, Syn.ClassishDeclaration _)
    | (_, Syn.FunctionDeclaration _) ->
      Stop patches
    | _ -> Continue patches
  in
  fold_syntax ~f:collect_patches ~init:[] root

let patches_of_nadable Summary.{ id; path_opt; _ } :
    ServerRefactorTypes.patch list =
  let open Option.Let_syntax in
  Option.value ~default:[]
  @@ let* path = path_opt in
     let+ source = File_provider.get_contents path in
     patches_of_nadable_id ~source ~path id

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syn)
module Tree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

let tree_of_code (path : Relative_path.t) (code : string) : Tree.t =
  let env = Full_fidelity_parser_env.make () in
  let source_text = Full_fidelity_source_text.make path code in
  Tree.make ~env source_text

let parse (path : Relative_path.t) (code_block : string) : Syn.t =
  let tree = tree_of_code path code_block in
  let root = Tree.root tree in
  Syn.from_positioned_syntax root

let hackfmt (path : Relative_path.t) (code : string) : string =
  code |> tree_of_code path |> Libhackfmt.format_tree

let _sp pos =
  Printf.sprintf
    "%d:%d-%d:%d"
    (fst @@ Pos.line_column pos)
    (snd @@ Pos.line_column pos)
    (fst @@ Pos.end_line_column pos)
    (snd @@ Pos.end_line_column pos)

let create_token ?text (kind : Full_fidelity_token_kind.t) : Syn.t =
  let text =
    Option.value text ~default:(Full_fidelity_token_kind.to_string kind)
  in
  {
    syntax =
      Token
        {
          kind;
          leading_text = "";
          trailing_text = "";
          token_data = Synthetic { text };
        };
    value = Synthetic;
  }

let needs_concrete_attribute_node : Syn.t =
  {
    syntax =
      ConstructorCall
        {
          constructor_call_type = create_token Name ~text:"__NeedsConcrete";
          constructor_call_left_paren = { syntax = Missing; value = Synthetic };
          constructor_call_argument_list =
            { syntax = Missing; value = Synthetic };
          constructor_call_right_paren = { syntax = Missing; value = Synthetic };
        };
    value = Synthetic;
  }

let add_attribute (methodish_attribute : Syn.t) ~(attribute : Syn.t) : Syn.t =
  match methodish_attribute.syntax with
  | Missing ->
    {
      methodish_attribute with
      syntax =
        OldAttributeSpecification
          {
            old_attribute_specification_left_double_angle =
              create_token LessThanLessThan;
            old_attribute_specification_attributes =
              { syntax = SyntaxList [attribute]; value = Synthetic };
            old_attribute_specification_right_double_angle =
              create_token GreaterThanGreaterThan;
          };
    }
  | OldAttributeSpecification
      ({
         old_attribute_specification_attributes =
           { syntax = SyntaxList attributes; _ };
         _;
       } as old_attribute_specification) ->
    let attributes =
      if
        List.exists attributes ~f:(fun attr ->
            String.is_substring (Syn.text attr) ~substring:"__NeedsConcrete")
      then
        attributes
      else
        let tail =
          if List.is_empty attributes then
            []
          else
            create_token Comma :: attributes
        in
        needs_concrete_attribute_node :: tail
    in
    {
      methodish_attribute with
      syntax =
        OldAttributeSpecification
          {
            old_attribute_specification with
            old_attribute_specification_attributes =
              { syntax = SyntaxList attributes; value = Synthetic };
          };
    }
  | _ -> methodish_attribute

let has_needs_concrete_attribute : Syn.t -> bool = function
  | {
      syntax =
        OldAttributeSpecification
          {
            old_attribute_specification_attributes =
              { syntax = SyntaxList attributes; value = Synthetic };
            _;
          };
      _;
    } ->
    List.exists attributes ~f:(function
        | { syntax = ConstructorCall { constructor_call_type = token; _ }; _ }
          ->
          String.equal (Syn.text token) "__NeedsConcrete"
        | _ -> false)
  | _ -> false

let rewrite_syntax
    (warnings : Codemod_sa_warning.t list)
    (path : Relative_path.t)
    (node : Syn.t) : Syn.t Rewriter.t =
  let warning_in_containing_pos containing_pos =
    (* suboptimal O(n) search but probably doesn't matter *)
    List.find warnings ~f:(fun { pos; warning_code } ->
        match warning_code with
        | Error_codes.Warning.CallNeedsConcrete
        | Error_codes.Warning.AbstractAccessViaStatic
        | Error_codes.Warning.UninstantiableClassViaStatic ->
          Pos.contains containing_pos pos
        | _ -> false)
  in
  match Syn.syntax node with
  | MethodishDeclaration ({ methodish_attribute; _ } as decl) -> begin
    match Syn.position path node with
    | Some containing_pos -> begin
      match warning_in_containing_pos containing_pos with
      (* apparently isn't reached? *)
      | Some error when has_needs_concrete_attribute methodish_attribute ->
        let () =
          Printf.eprintf
            "Found an error about missing __NeedsConcrete. But the attribute already has __NeedsConcrete. %s \n"
            (Pos.print_verbose_relative error.pos)
        in
        Rewriter.Keep
      | Some _ ->
        Rewriter.Replace
          {
            node with
            syntax =
              MethodishDeclaration
                {
                  decl with
                  methodish_attribute =
                    add_attribute
                      methodish_attribute
                      ~attribute:needs_concrete_attribute_node;
                };
          }
      | None -> Rewriter.Keep
    end
    | None -> Rewriter.Keep
  end
  | _ -> Rewriter.Keep

let rewrite
    (path : Relative_path.t)
    (warnings : Codemod_sa_warning.t list)
    (code : string) : string =
  let node = parse path code in
  Rewriter.rewrite_post (rewrite_syntax warnings path) node
  |> Syn.text
  |> hackfmt path

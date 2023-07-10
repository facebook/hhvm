(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree.WithSyntax (Syn)

let apply_patches_to_string old_content (patches : ServerRenameTypes.patch list)
    : string =
  let buf = Buffer.create (String.length old_content) in
  let patch_list =
    List.sort ~compare:ServerRenameTypes.compare_result patches
  in
  ServerRenameTypes.write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

(** given the source text of a block, apply `f` to source text wrapped such that it's
  a valid Hack program (with `<?hh` and all).
  Then strip off the wrapping (`<?hh` and all).
  *)
let with_wrapped_source_text ~block_source_text ~(f : string -> 'a * string) :
    'a * string =
  let unwrap (wrapped_source_text : string) : string =
    wrapped_source_text
    |> String.split_lines
    |> (fun lines -> List.drop lines 2) (* drop "<?hh" and "function foo ..." *)
    |> List.drop_last_exn
       (* we wrapped s.t. the string is guaranteed to have a last line *)
    |> String.concat ~sep:"\n"
  in
  let wrapped =
    Printf.sprintf "<?hh\nfunction foo(): void {\n%s\n}" block_source_text
  in
  let (res, rewritten_block_wrapped) = f wrapped in
  let rewritten_block = unwrap rewritten_block_wrapped in
  (res, rewritten_block)

let tree_of_text path source_text =
  Full_fidelity_source_text.make path source_text |> PositionedTree.make

(**
Re the `start_indent` param:
We remember `start_indent` so we can restore it after formatting.
In the following, `start_indent` is `2`:

```
if (1 < 2) {
  // start
  $x = 3;
  //end
}
```
*)
let format_block path block_source_text ~start_indent_amount =
  let add_indent text =
    let indent = String.make start_indent_amount ' ' in
    let inner_indent =
      (* adjusted because the autoformatter already indents once, due to our
         trick of wrapping the code-to-format in a function *)
      String.make (start_indent_amount - Format_env.(default.indent_width)) ' '
    in
    text
    |> String.split_lines
    |> List.map ~f:(Printf.sprintf "%s%s" inner_indent)
    |> String.concat ~sep:"\n"
    |> fun text -> Printf.sprintf "%s\n%s" text indent
  in
  with_wrapped_source_text ~block_source_text ~f:(fun wrapped_source_text ->
      ((), Libhackfmt.format_tree @@ tree_of_text path wrapped_source_text))
  |> snd
  |> add_indent

let rewrite_block r path block_source_text ~return_var_raw_name :
    Inline_method_rename.t * string =
  with_wrapped_source_text ~block_source_text ~f:(fun wrapped_source_text ->
      let module Ff_rewriter = Full_fidelity_rewriter.WithSyntax (Syn) in
      let fold node ((r, patches) as acc) =
        let acc =
          match Syn.syntax node with
          | Syn.VariableExpression { variable_expression } ->
            let var = Syn.text variable_expression in
            (match Syn.position_exclusive path node with
            | Some pos ->
              let (r, var) = Inline_method_rename.rename r var in
              let patch =
                ServerRenameTypes.Replace
                  ServerRenameTypes.{ pos = Pos.to_absolute pos; text = var }
              in
              (r, patch :: patches)
            | None -> acc)
          | Syn.ReturnStatement { return_keyword; _ } ->
            (match Syn.position_exclusive path return_keyword with
            | Some pos ->
              let (r, return_var) =
                Inline_method_rename.rename r return_var_raw_name
              in
              let text = Printf.sprintf "%s = " return_var in
              let patch =
                ServerRenameTypes.Replace
                  ServerRenameTypes.{ pos = Pos.to_absolute pos; text }
              in
              (r, patch :: patches)
            | None -> acc)
          | _ -> acc
        in
        (acc, Ff_rewriter.Result.Keep)
      in
      let root = tree_of_text path wrapped_source_text |> PositionedTree.root in
      let (r, patches) =
        fst @@ Ff_rewriter.aggregating_rewrite_post fold root (r, [])
      in
      let rewritten = apply_patches_to_string wrapped_source_text patches in
      (r, rewritten))

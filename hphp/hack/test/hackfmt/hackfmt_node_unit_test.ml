(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

module SourceText = Full_fidelity_source_text
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module EditableSyntax = Full_fidelity_editable_syntax
open OUnit

let get_expr src =
  Full_fidelity_syntax_kind.(
    let prefix = "<?hh\n" in
    prefix ^ src
    |> SourceText.make Relative_path.(create Dummy "<format_node>")
    |> SyntaxTree.make
    |> SyntaxTransforms.editable_from_positioned
    |> (fun node -> EditableSyntax.parentage node (String.length prefix))
    |> List.find (fun node -> EditableSyntax.kind node = ExpressionStatement))

let assert_formatting
    ~src
    ?(indent = 0)
    ?(line_width = 20)
    ?(indent_width = 2)
    ?(indent_with_tabs = false)
    ~exp
    () =
  let config =
    Format_env.{ default with line_width; indent_width; indent_with_tabs }
  in
  let node = get_expr src in
  let formatted = Libhackfmt.format_node ~config ~indent node in
  assert_equal exp formatted

let range_test_suite =
  "hackfmt_node"
  >::: [
         "simple"
         >:: assert_formatting ~src:"$x = f($a,$b);" ~exp:"$x = f($a, $b);\n";
         "indented"
         >:: assert_formatting
               ~src:"$x = f($a,$b);"
               ~indent:1
               ~exp:"  $x = f($a, $b);\n";
         "indented_and_wrapped"
         >:: assert_formatting
               ~src:"$x = f($a,$b);"
               ~indent:1
               ~line_width:10
               ~exp:("  $x = f(\n" ^ "    $a,\n" ^ "    $b,\n" ^ "  );\n");
         "indented_one_space"
         >:: assert_formatting
               ~src:"$x = f($a,$b);"
               ~indent:1
               ~line_width:10
               ~indent_width:1
               ~exp:(" $x = f(\n" ^ "  $a,\n" ^ "  $b,\n" ^ " );\n");
         "indented_with_tabs"
         >:: assert_formatting
               ~src:"$x = f($a,$b);"
               ~indent:1
               ~line_width:10
               ~indent_with_tabs:true
               ~exp:("\t$x = f(\n" ^ "\t\t$a,\n" ^ "\t\t$b,\n" ^ "\t);\n");
         "string_literal"
         >:: assert_formatting
               ~src:"$x = f($a,'foo\nbar');"
               ~indent:1
               ~exp:"  $x = f($a, 'foo\nbar');\n";
       ]

let _ = run_test_tt_main range_test_suite

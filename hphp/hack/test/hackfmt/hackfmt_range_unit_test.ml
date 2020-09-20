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
open OUnit

let sum = "<?hh
function sum($a, $b) {
  return $a + $b;
}"

let format_range src range =
  let path = Relative_path.(create Dummy "<format_range>") in
  src
  |> SourceText.make path
  |> SyntaxTree.make
  |> Libhackfmt.format_range range

let substr s (st, ed) = String.sub s st (ed - st)

let assert_range_formats_to ~src ~rng ~exp () =
  let (range, expected_substring) = rng in
  (* As a sanity check, and a convenience to the reader, require tests to
   * provide the text of the range they are formatting. *)
  assert_equal
    ~msg:
      (Printf.sprintf
         "Provided range (%d,%d) doesn't match source text substring"
         (fst range)
         (snd range))
    expected_substring
    (substr src range);

  (* Perform the actual test. *)
  assert_equal exp (format_range src range)

let range_test_suite =
  "hackfmt_range"
  >::: [
         "first_token_on_line"
         >:: assert_range_formats_to
               ~src:sum
               ~rng:((30, 36), "return")
               ~exp:"  return";
         "token_with_leading_indentation"
         >:: assert_range_formats_to
               ~src:sum
               ~rng:((28, 36), "  return")
               ~exp:"  return";
         "trailing_newline"
         >:: assert_range_formats_to ~src:sum ~rng:((26, 28), "{\n") ~exp:"{\n";
         "trailing_newline_omitted"
         >:: assert_range_formats_to ~src:sum ~rng:((26, 27), "{") ~exp:"{\n";
         "trailing_newline_character"
         >:: assert_range_formats_to ~src:sum ~rng:((27, 28), "\n") ~exp:"";
         "leading_whitespace"
         >:: assert_range_formats_to ~src:sum ~rng:((28, 30), "  ") ~exp:"";
         "inline_whitespace"
         >:: assert_range_formats_to ~src:sum ~rng:((36, 37), " ") ~exp:"";
         "end_of_file"
         >:: assert_range_formats_to ~src:sum ~rng:((46, 47), "}") ~exp:"}\n";
         "operator_surrounding_whitespace"
         >:: assert_range_formats_to
               ~src:"$a + $b"
               ~rng:((2, 5), " + ")
               ~exp:"+";
         "operator_without_whitespace"
         >:: assert_range_formats_to ~src:"$a + $b" ~rng:((3, 4), "+") ~exp:"+";
         "up_to_blank_line"
         >:: assert_range_formats_to
               ~src:"a;\n\nb;"
               ~rng:((0, 3), "a;\n")
               ~exp:"a;\n";
         "blank_line_at_end"
         >:: assert_range_formats_to
               ~src:"a;\n\nb;"
               ~rng:((0, 4), "a;\n\n")
               ~exp:"a;\n\n";
         "blank_line_at_start"
         >:: assert_range_formats_to
               ~src:"a;\n\nb;"
               ~rng:((3, 6), "\nb;")
               ~exp:"\nb;\n";
         "comma_at_start_of_range"
         >:: assert_range_formats_to
               ~src:"f($x, $y)"
               ~rng:((4, 8), ", $y")
               ~exp:", $y";
         (* When the formatter inserts a trailing comma, ensure that it is printed
          * only if the atom preceding it is printed. *)
         "trailing_comma_inserted_at_start_of_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y)")
               ~rng:((106, 107), ")")
               ~exp:")\n";
         "trailing_comma_inserted_at_end_of_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y)")
               ~rng:((104, 106), "$y")
               ~exp:"  $y,\n";
         (* If a trailing comma was already present outside the range, don't include
          * it in the formatted output. *)
         "trailing_comma_existed_at_start_of_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y,)")
               ~rng:((107, 108), ")")
               ~exp:")\n";
         "trailing_comma_existed_at_end_of_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y,)")
               ~rng:((104, 106), "$y")
               ~exp:"  $y";
         "trailing_comma_included_at_start_of_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y,)")
               ~rng:((106, 108), ",)")
               ~exp:",\n)\n";
         "trailing_comma_included_at_end_of_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y,)")
               ~rng:((104, 107), "$y,")
               ~exp:"  $y,\n";
         "trailing_comma_is_entire_range"
         >:: assert_range_formats_to
               ~src:("<?hh\n" ^ String.make 95 'f' ^ "($x,$y,)")
               ~rng:((106, 107), ",")
               ~exp:",\n";
         "deleted_trailing_comma_is_entire_range"
         >:: assert_range_formats_to ~src:"f($x,$y,)" ~rng:((7, 8), ",") ~exp:"";
       ]

let _ = run_test_tt_main range_test_suite

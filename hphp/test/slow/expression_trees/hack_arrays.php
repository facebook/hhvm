<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  // Basic vec
  $et = ExampleDsl`vec[1, 2, 3]`;
  print_et($et);

  // Vec with strings
  $et2 = ExampleDsl`vec["a", "b", "c"]`;
  print_et($et2);

  // Dict with string keys
  $et3 = ExampleDsl`dict["a" => 1, "b" => 2]`;
  print_et($et3);

  // Dict with int keys
  $et4 = ExampleDsl`dict[0 => "a", 1 => "b"]`;
  print_et($et4);

  // Empty vec
  $et5 = ExampleDsl`vec[]`;
  print_et($et5);

  // Empty dict
  $et6 = ExampleDsl`dict[]`;
  print_et($et6);

  // Nested vecs
  $et7 = ExampleDsl`vec[vec[1, 2], vec[3, 4]]`;
  print_et($et7);

  // Vec in lambda
  $et8 = ExampleDsl`(ExampleInt $x) ==> vec[$x, 1, 2]`;
  print_et($et8);

  // Dict in lambda with string key
  $et9 = ExampleDsl`(ExampleString $k, ExampleInt $v) ==> dict[$k => $v]`;
  print_et($et9);

  // Dict in lambda with int key
  $et10 = ExampleDsl`(ExampleInt $k, ExampleString $v) ==> dict[$k => $v]`;
  print_et($et10);

  // Keyset with int elements
  print_et(ExampleDsl`keyset[1, 2, 3]`);

  // Keyset with string elements
  print_et(ExampleDsl`keyset["a", "b", "c"]`);

  // Empty keyset
  print_et(ExampleDsl`keyset[]`);

  // Keyset in lambda
  print_et(ExampleDsl`(ExampleInt $x) ==> keyset[$x, 1, 2]`);

  // Keyset string in lambda
  print_et(ExampleDsl`(ExampleString $s) ==> keyset[$s, "a", "b"]`);
}

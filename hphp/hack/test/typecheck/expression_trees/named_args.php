<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

$x = ExampleDsl`() ==> {
  $x = 1;
  // Should be a parse error before we even type check.
  return foo(x=$x);
}`;

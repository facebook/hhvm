<?hh
// This test is sensitive to what gets jitted vs. what gets interpreted.
// The .opts file for this test sets JitGlobalTranslationLimit in a way
// such that the second foreach loop's IterInit instruction gets jitted
// but its IterNext instruction gets interpreted.
function main() {
  $y = 'z';
  $z = 1;
  $x = Vector {1, 2, 3};
  $x[] = 4;
  foreach ($x as $v) { $$y = 1; }
  $x = ImmVector {1, 2, 3};
  foreach ($x as $v) { $$y = 1; }
  echo "Done\n";
}
main();

<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Second bound refines the first by adding a field
function test_refine_adds_field<
  T as shape('x' => int, ...) as shape('x' => int, 'z' => bool, ...),
>(T $s): void {
  shape('x' => $x, 'z' => $z, ...) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<bool>($z);
}

function test_tuple<T as (int, string) as (int, string)>(T $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}

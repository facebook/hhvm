<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type CT = vec<CT> | int;

class Bar {}

function expect_CT(CT $x): void {}

function test(
  vec<Bar> $vec_bar,        // should fail
  vec<int> $vec_int,        // should succeed
  vec<string> $vec_str,     // should fail
  vec<vec<int>> $vec_vec,   // should succeed
  vec<vec<Bar>> $vec_vec_b, // should fail
  int $i,                   // should succeed
  Bar $bar,                 // should fail
  CT $ct,                   // should succeed
  vec<CT> $vct,             // should succeed
): void {
  expect_CT($vec_bar);
  expect_CT($vec_int);
  expect_CT($vec_str);
  expect_CT($vec_vec);
  expect_CT($vec_vec_b);
  expect_CT($i);
  expect_CT($bar);
  expect_CT($ct);
  expect_CT($vct);
}

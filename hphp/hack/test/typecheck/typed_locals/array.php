<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

function expect_vec_n(vec<nothing> $v): void {}

function f(): void {
  let $v: vec<arraykey> = vec[];
  expect_vec_n($v);
  $v[] = 1;
  hh_expect_equivalent<vec<int>>($v);
  $v[] = "2";
  hh_expect_equivalent<vec<(string | int)>>($v);
}

function g(): void {
  let $v: Vector<arraykey> = Vector {};
  $v[] = 1;
  $v[] = "2";
}

<?hh


function expect_generic_splat<T as (mixed...)>(... T $x): void {}
function expect_fixed_splat(... (mixed...) $x): void {}

function make_vec(): vec<mixed> {
  return vec[2];
}
function test1(vec<mixed> $vm): void {
  expect_fixed_splat(...$vm);
  expect_generic_splat(...$vm);
  $vm2 = make_vec();
  expect_fixed_splat(...$vm2);
  expect_generic_splat(...$vm2);
}

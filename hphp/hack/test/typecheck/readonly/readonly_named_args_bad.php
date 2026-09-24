<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>
//
function readonly_mutable_target(int $m, named int $n): void {}

function test_readonly_named_binding_bad(): void {
  $ro = readonly 1;
  // A readonly positional argument is rejected against mutable $m
  // in either order: it must not be associated with named $n.
  readonly_mutable_target($ro, n = 2);
  readonly_mutable_target(n = 2, $ro);
}

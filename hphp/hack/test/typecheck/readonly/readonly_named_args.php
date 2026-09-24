<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>
//
function readonly_named_target(readonly int $r, named int $z): void {}

function test_readonly_named_binding(): void {
  // Both orders are legal: the positional readonly argument binds to $r.
  readonly_named_target(readonly 1, z = 2);
  readonly_named_target(z = 2, readonly 1);

  $ro = readonly 1;
  readonly_named_target($ro, z = 2);
  readonly_named_target(z = 2, $ro);
}

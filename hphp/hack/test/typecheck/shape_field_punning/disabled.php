<?hh

function test_without_feature(): void {
  $foo = 1;
  // This should fail because the feature is not enabled
  $s = shape($foo);
}

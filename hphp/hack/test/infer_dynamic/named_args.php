<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function test_named_dynamic(dynamic $d): void {
  $d(n = 1);
}

function test_mixed_dynamic(dynamic $d): void {
  $d(42, name = "hello");
}

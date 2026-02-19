<?hh
// test_feature defaults to Unstable, not enabled → parser error
<<__EntryPoint>>
function main(): void {
  echo __EXPERIMENTAL_TEST_FEATURE_STATUS__ . "\n";
}

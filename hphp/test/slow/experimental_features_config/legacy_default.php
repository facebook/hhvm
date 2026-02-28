<?hh
// Without configuring the new flags, legacy mode applies
// test_feature has Unstable as its hardcoded default
<<file:__EnableUnstableFeatures('test_feature')>>
<<__EntryPoint>>
function main(): void {
  echo __EXPERIMENTAL_TEST_FEATURE_STATUS__ . "\n";
}

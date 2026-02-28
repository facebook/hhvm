<?hh
// New config system with consider_unspecified_released=false (default)
// test_feature not in map, so defaults to Unstable
<<file:__EnableUnstableFeatures('test_feature')>>
<<__EntryPoint>>
function main(): void {
  echo __EXPERIMENTAL_TEST_FEATURE_STATUS__ . "\n";
}

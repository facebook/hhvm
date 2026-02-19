<?hh
// New config system with consider_unspecified_released=true
// test_feature not in map, so defaults to OngoingRelease
<<__EntryPoint>>
function main(): void {
  echo __EXPERIMENTAL_TEST_FEATURE_STATUS__ . "\n";
}

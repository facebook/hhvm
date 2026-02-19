<?hh
// test_feature set to Preview, enabled via file attribute
<<file:__EnableUnstableFeatures('test_feature')>>
<<__EntryPoint>>
function main(): void {
  echo __EXPERIMENTAL_TEST_FEATURE_STATUS__ . "\n";
}

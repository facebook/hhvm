<?hh
// test_feature defaults to Unstable, but .hhvmconfig.hdf overrides to OngoingRelease
<<__EntryPoint>>
function main(): void {
  echo __EXPERIMENTAL_TEST_FEATURE_STATUS__ . "\n";
}

<?hh

// 1. function access_test_package_override_2() is in a file in intern
//    that has a PackageOverride
// 2. intern is excluded via --exclude-dir
//
// The call to access_test_package_override should succeed because
// PackageOverride can pull files from --exclude-dir

<<__EntryPoint>>
function exclude_dir_package_override_1(): void {
  base();
  access_test_package_override_2(); // this should succeed
}

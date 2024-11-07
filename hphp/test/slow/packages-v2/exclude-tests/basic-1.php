<?hh

// the call to access_test_package_override should succeed because
// basic-1-test.inc.php has a package override to a package (foo)
// in the active deployment, despite __tests__ being in the
// exclude paths

<<__EntryPoint>>
function main_basic_1(): void {
  base();
  access_test_package_override(); // this should succeed
}

<?hh

// 1. function access_test_package_override() is in __tests__
// 2. __tests__ is excluded via --exclude-pattern
//
// The call to access_test_package_override must fail because
// --exclude-pattern cannot be overridden

<<__EntryPoint>>
function main_basic_1(): void {
  base();
  access_test_package_override(); // this should fail
}

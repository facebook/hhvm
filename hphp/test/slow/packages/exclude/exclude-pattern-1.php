<?hh

// access_test is defined in __tests__
// __tests__ is excluded from the build with --exclude-pattern

<<__EntryPoint>>
function main_basic_2(): void {
  base();
  access_test(); // this should fail
}

//// foo.php
<?hh
<<file:__PackageOverride('pkg4')>>
// package pkg4
// since pkg4 includes no paths we are sure that bar.php
// is not in the same package by accident
function foo(): void {}

//// __tests__/bar.php
<?hh
// package test
function test(): void {
  foo();  // this is allowed as tests can call anything
}

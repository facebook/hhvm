//// pkg4/foo.php
<?hh
// package pkg4 by path; bar.php lives elsewhere so it is not in the same
// package by accident
function foo(): void {}

//// /__tests__/bar.php
<?hh
// package test
function test(): void {
  foo();  // this is allowed as tests can call anything
}

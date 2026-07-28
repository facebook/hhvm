//// /__tests__/foo.php
<?hh
// package pkg1
function test(): void {}

//// pkg4/foo.php
<?hh
// package pkg4 by path (pkg4 does not include pkg1)
function foo(): void {
  test (); // but ok because calls to __tests__ are allowed
}

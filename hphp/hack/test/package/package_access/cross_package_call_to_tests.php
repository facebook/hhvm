//// /__tests__/foo.php
<?hh
// package pkg1
function test(): void {}

//// foo.php
<?hh
// package pkg4
<<file: __PackageOverride('pkg4')>>
// TODO: this should be an error as pkg4 does not include pkg1
function foo(): void {
  test (); // but ok because calls to __tests__ are allowed
}

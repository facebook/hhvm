//// __tests__/foo.php
<?hh
// package pkg1
function test(): void {}

//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
// TODO: this should be an error as arbitrary packages should not
// be allowed to call into __tests__
function foo(): void {
  test (); // ok because calls to __tests__ are allowed
}

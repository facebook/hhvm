//// /__tests__/foo.php
<?hh
// package pkg1
<<__RequirePackage('pkg2')>>
function bar(): void {}

<<__SoftRequirePackage('pkg2')>>
function bar_soft(): void {}

class C {
  <<__RequirePackage('pkg2')>>
  public function foo(): void {}

  <<__SoftRequirePackage('pkg2')>>
  public function foo_soft(): void {}
}


//// foo.php
<?hh
// package pkg4
<<file: __PackageOverride('pkg4')>>
// all these should be an error as pkg4 does not include pkg1
function test(): void {
  bar(); // but ok because calls to __tests__ are allowed
  bar_soft();
  (new C())->foo();
  (new C())->foo_soft();
}

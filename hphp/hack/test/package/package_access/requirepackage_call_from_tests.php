//// foo.php
<?hh
// package pkg1, pkg2 includes pkg1

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

//// /__tests__/bar.php
<?hh
// package pkg1, but in __tests__ directory
function test(): void {
  (new C())->foo();  // all these are allowed as tests can call anything
  bar();
  (new C())->foo_soft();
  bar_soft();
  
}

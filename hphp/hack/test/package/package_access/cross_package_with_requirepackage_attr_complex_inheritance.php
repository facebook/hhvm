//// a.php
<?hh
// package pkg1

class A {
  public function foo(): void {}
}
<<__RequirePackage("pkg2")>>
function foo(): void {}

function takes_fun((function(): void) $f) : void {
  $f();
}

function test(): void {
  if (package pkg2) {
    takes_fun(foo<> /* we can resolve the pointer here because of the if package */);
  }
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

class B extends A { // ok because pkg2 includes pkg1
  <<__RequirePackage("pkg1")>> // package requirements must strictly include the file package
  public function foo(): void {}
}

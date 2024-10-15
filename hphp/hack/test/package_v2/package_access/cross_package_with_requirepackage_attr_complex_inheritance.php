//// a.php
<?hh
// package pkg1
<<file: __EnableUnstableFeatures('require_package')>>

class A {
  public function foo(): void {}
}
<<__RequirePackage("pkg2")>>
function foo(): void {}

// can't pass cross package functions here
function takes_fun((function(): void) $f) : void {
  $f();
}

function test(): void {
  if (package pkg2) {
    takes_fun(foo<>); // error
  }
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
<<file: __EnableUnstableFeatures('require_package')>>

class B extends A { // ok because pkg2 includes pkg1
  <<__RequirePackage("pkg1")>> // this will unnecessarily error, but it's also redundant: pkg1 is always included
  public function foo(): void {} // TODO: give a better error message instead of erroring
}

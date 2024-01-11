//// module_a.php
<?hh
new module a {}     // package pkg1

//// module_b.php
<?hh
new module b.b1 {}  // package pkg2

//// a.php
<?hh

module a;

class A {
  public function foo(): void {}
}
<<__CrossPackage("pkg2")>>
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
module b.b1
class B extends A { // ok because pkg2 includes pkg1
  <<__CrossPackage("pkg1")>> // this will unnecessarily error, but it's also redundant: pkg1 is always included
  public function foo(): void {} // TODO: give a better error message instead of erroring
}

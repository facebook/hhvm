<?hh
// package foo

class A {
  // Ok - package bar is in the same deployment as package foo
  <<__RequirePackage("bar")>>
  static function bar() : void {
    var_dump("in A::bar");
  }
}

// OK - package softbar is soft-deployed in the same deployment as package foo
<<__RequirePackage("softbar")>>
function a(): void {
  var_dump("in a");
}

<<__RequirePackage("bat")>>
// ERROR - package bat is not deployed in the same deployment as package foo
function b(): void {
  var_dump("in b");
}


<<__Entrypoint>>
function main(): void {
  A::bar();
  a();
  b();
}

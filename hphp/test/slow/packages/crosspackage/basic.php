<?hh

module a; // package foo

class A {
  // Ok - package bar is in the same deployment as package foo
  <<__CrossPackage("bar")>>
  static function bar() : void {
    var_dump("in A::bar");
  }
}

// Ok - package bat is soft-deployed in the same deployment as package foo
<<__CrossPackage("bat")>>
function a(): void {
  var_dump("in a");
}

<<__Entrypoint>>
function main(): void {
  A::bar();
  a();
}

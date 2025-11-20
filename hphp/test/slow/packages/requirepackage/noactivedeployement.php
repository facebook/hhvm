<?hh
// package bar

class A1 {
  // Ok - package bar is in the same deployment as package foo
  <<__RequirePackage("bar")>>
  static function bar() : void {
    var_dump("in A1::bar");
  }
}

// OK - package softbar is soft-deployed in the same deployment as package foo
<<__RequirePackage("softbar")>>
function a1(): void {
  var_dump("in a1");
}

<<__RequirePackage("bat")>>
// ERROR - package bat is not deployed in the same deployment as package foo
function b1(): void {
  var_dump("in b1");
}


<<__Entrypoint>>
function main1(): void {
  A1::bar();
  a1();
  b1();
}

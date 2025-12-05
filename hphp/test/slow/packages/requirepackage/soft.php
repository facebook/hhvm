<?hh
// package foo

class ASoft {
  // Ok - package bar is in the same deployment as package foo
  <<__SoftRequirePackage("bar")>>
  static function bar() : void {
    var_dump("in A::bar");
  }

<<__SoftRequirePackage("bat")>>
//  LOG - package bat is not deployed in the same deployment as package foo
public static function batMemSoft(): void {
  var_dump("in bMem");
}
}

<<__SoftRequirePackage("bat")>>
//  LOG - package bat is not deployed in the same deployment as package foo
function b_soft(): void {
  var_dump("in b");
}


<<__Entrypoint>>
function soft_main(): void {
  ASoft::bar();
  b_soft();
  ASoft::batMemSoft();
}

<?hh
// package foo

class ASoftUnsampled {
  // Ok - package bar is in the same deployment as package foo
  <<__SoftRequirePackage("bar", 0)>>
  static function bar() : void {
    var_dump("in A::bar");
  }

<<__SoftRequirePackage("bat", 0)>>
//  LOG - package bat is not deployed in the same deployment as package foo
public static function batMemSoft(): void {
  var_dump("in bMem");
}
}

<<__SoftRequirePackage("bat", 0)>>
//  LOG - package bat is not deployed in the same deployment as package foo
function b_soft_unsampled(): void {
  var_dump("in b");
}


<<__Entrypoint>>
function soft_unsampled_main(): void {
  ASoftUnsampled::bar();
  b_soft_unsampled();
  ASoftUnsampled::batMemSoft();
}

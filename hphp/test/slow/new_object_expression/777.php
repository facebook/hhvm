<?hh

class A {
}
 class B extends A {
 static function foo() {
 return new parent();
}
 }

 <<__EntryPoint>>
function main_777() {
var_dump(B::foo());
}

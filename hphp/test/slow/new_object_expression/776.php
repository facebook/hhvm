<?hh

class A {
}
 class B extends A {
 static function foo() {
 return new self();
}
 }

 <<__EntryPoint>>
function main_776() {
var_dump(B::foo());
}

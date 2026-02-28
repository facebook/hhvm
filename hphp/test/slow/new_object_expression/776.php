<?hh

class A {
}
 class B extends A {
 static function foo() :mixed{
 return new self();
}
 }

 <<__EntryPoint>>
function main_776() :mixed{
var_dump(B::foo());
}

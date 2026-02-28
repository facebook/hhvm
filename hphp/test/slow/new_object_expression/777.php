<?hh

class A {
}
 class B extends A {
 static function foo() :mixed{
 return new parent();
}
 }

 <<__EntryPoint>>
function main_777() :mixed{
var_dump(B::foo());
}

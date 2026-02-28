<?hh

class A {
 static private $foo = 11;
 }
 class B extends A {
}
 class C extends B {
 static public $foo;
}


 <<__EntryPoint>>
function main_675() :mixed{
var_dump(C::$foo);
}

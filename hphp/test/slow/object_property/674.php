<?hh

class A {
 static protected $foo = 11;
 }
 class B extends A {
}
 class C extends B {
 static public $foo;
}


 <<__EntryPoint>>
function main_674() {
var_dump(C::$foo);
}

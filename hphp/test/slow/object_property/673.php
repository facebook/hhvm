<?hh

class A {
 static protected $foo = 11;
   static function foo() :mixed{
 var_dump(A::$foo);
}
}
 class B extends A {
 static public $foo;
}

 <<__EntryPoint>>
function main_673() :mixed{
var_dump(B::$foo);
 B::$foo = 123;
 A::foo();
}

<?hh

class A {
 public static $a = 1;
}
 class B extends A {
 public static $a = 2;
}

 <<__EntryPoint>>
function main_1269() :mixed{
var_dump(B::$a);
}

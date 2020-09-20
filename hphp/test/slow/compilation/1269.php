<?hh

class A {
 static $a = 1;
}
 class B extends A {
 static $a = 2;
}

 <<__EntryPoint>>
function main_1269() {
var_dump(B::$a);
}

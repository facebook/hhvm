<?hh
function foo($a) {
 print $a;
}
 class A {
 function __construct() {
}
}


<<__EntryPoint>>
function main_1710() {
error_reporting(E_ALL & ~E_NOTICE);
 $obj = new A(foo(10));
}

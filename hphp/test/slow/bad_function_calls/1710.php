<?hh
function foo($a) :mixed{
 print $a;
}
 class A {
 function __construct() {
}
}


<<__EntryPoint>>
function main_1710() :mixed{
error_reporting(E_ALL & ~E_NOTICE);
 $obj = new A(foo(10));
}

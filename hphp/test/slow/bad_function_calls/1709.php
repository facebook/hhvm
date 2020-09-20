<?hh
class A {
 function __construct() {
}
}


<<__EntryPoint>>
function main_1709() {
error_reporting(E_ALL & ~E_NOTICE);
 $obj = new A(10);
}

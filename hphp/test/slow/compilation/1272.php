<?hh

class A {
 function __construct($a) {
 var_dump($a);
}
 }
 class B extends A {
}

 <<__EntryPoint>>
function main_1272() :mixed{
$a = new B('test');
}

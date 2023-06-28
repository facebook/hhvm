<?hh

class A {
 }

<<__EntryPoint>>
function main_687() :mixed{
$a = new A();
$f = 20;
$a->$f = 100;
var_dump($a);
unset($a->$f);
var_dump($a);
}

<?hh

class A {
 function f($a) :mixed{
 var_dump($a === null);
 }
 }

<<__EntryPoint>>
function main_234() :mixed{
$a = true;
 $a = new A();
$a->f(vec[]);
}

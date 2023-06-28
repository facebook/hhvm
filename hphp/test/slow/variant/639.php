<?hh

class a {
 public $var2 = 1;
 public $var1;
 }
class b extends a {
 public $var2;
 }
function f() :mixed{
 $obj1 = new b();
 var_dump($obj1);
 $obj1->var1 = 1;
 }

<<__EntryPoint>>
function main_639() :mixed{
f();
}

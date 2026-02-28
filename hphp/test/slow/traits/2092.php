<?hh
trait t1 {
 public $x = INIT1;
 }
class c {
 use t1;
 public $y = INIT2;
 }

 const INIT1 = 123;
 const INIT2 = 456;

<<__EntryPoint>>
function main_2092() :mixed{
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
}

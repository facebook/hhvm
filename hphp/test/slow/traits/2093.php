<?hh
trait t2 {
 public $x = INIT1;
 }
trait t1 {
 use t2;
 }
class c {
 use t1;
 }


const INIT1 = 123;
<<__EntryPoint>>
function main_2093() :mixed{
$obj = new c;
var_dump($obj->x);
}

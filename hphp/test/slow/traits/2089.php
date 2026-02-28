<?hh
trait t1 {
 public $x = INIT1;
 }
class c {
 use t1;
 }

const INIT1 = 123;

<<__EntryPoint>>
function main_2089() :mixed{
$obj = new c;
var_dump($obj->x);
}

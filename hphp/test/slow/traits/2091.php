<?hh
trait t1 {
}
class c {
 use t1;
 public $x = INIT1;
 }


const INIT1 = 123;
<<__EntryPoint>>
function main_2091() :mixed{
$obj = new c;
var_dump($obj->x);
}

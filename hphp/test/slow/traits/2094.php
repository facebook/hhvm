<?hh
trait t2 {
 public $y = INIT2;
 }
trait t1 {
 use t2;
 public $x = INIT1;
 }
class c {
 use t1;
 }


const INIT1 = 123;
const INIT2 = 456;
<<__EntryPoint>>
function main_2094() :mixed{
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
}

<?hh
trait t1 {
 public $x = INIT1;
 }
trait t2 {
 public $y = INIT2;
 }
trait t {
 use t1, t2;
 }
class c {
 use t;
 }


const INIT1 = "1";
const INIT2 = "2";
<<__EntryPoint>>
function main_2090() :mixed{
$obj = new c;
var_dump($obj->x);
var_dump($obj->y);
}

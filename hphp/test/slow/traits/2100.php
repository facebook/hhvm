<?hh
trait t2 {
 static public $x = INIT1;
 }
trait t1 {
 use t2;
 }
class c {
 use t1;
 }

const INIT1 = 123;

<<__EntryPoint>>
function main_2100() :mixed{
var_dump(c::$x);
}

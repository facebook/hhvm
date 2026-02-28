<?hh
trait t1 {
 static public $x = INIT1;
 }
class c {
 use t1;
 }


const INIT1 = 123;
<<__EntryPoint>>
function main_2095() :mixed{
var_dump(c::$x);
}

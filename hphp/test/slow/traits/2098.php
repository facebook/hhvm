<?hh
trait t1 {
}
class c {
 use t1;
 static public $x = INIT1;
 }

const INIT1 = 123;

<<__EntryPoint>>
function main_2098() :mixed{
var_dump(c::$x);
}

<?hh
trait t2 {
 static public $y = INIT2;
 }
trait t1 {
 use t2;
 static public $x = INIT1;
 }
class c {
 use t1;
 }


const INIT1 = 123;
const INIT2 = 456;
<<__EntryPoint>>
function main_2101() :mixed{
var_dump(c::$x);
var_dump(c::$y);
}

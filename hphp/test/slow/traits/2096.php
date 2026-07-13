<?hh
trait t1 {
 static public $x = INIT1;
 }
class c {
  use t1;
 }


const INIT1 = 123;
<<__EntryPoint>>
function main_2096() :mixed{
$t=c::$x; c::$x++; var_dump($t);
$t=t1::$x; t1::$x++; var_dump($t);
$t=c::$x; c::$x++; var_dump($t);
$t=t1::$x; t1::$x++; var_dump($t);
}

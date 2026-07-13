<?hh
class d {
 static public $x = INIT1;
 }
class e extends d {
}


const INIT1 = 1000;
<<__EntryPoint>>
function main_699() :mixed{
$t=d::$x; d::$x++; var_dump($t);
$t=e::$x; e::$x++; var_dump($t);
}

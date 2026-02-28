<?hh
class d {
 static public $x = INIT1;
 }
class e extends d {
}


const INIT1 = 1000;
<<__EntryPoint>>
function main_699() :mixed{
var_dump(d::$x++);
var_dump(e::$x++);
}

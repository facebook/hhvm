<?hh


<<__EntryPoint>>
function main_1586() :mixed{
$s = " ";
$a = "hello";
$a .= $s;
$a .= "world";
var_dump($a);
$a = "a";
$a .= "b";
$a .= $a;
var_dump($a);
$a = 3;
echo 0 + HH\Lib\Legacy_FIXME\cast_for_arithmetic("1$a");
}

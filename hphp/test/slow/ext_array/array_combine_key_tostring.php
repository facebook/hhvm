<?hh

class Foo { function __toString() { return 'Hello'; } }

<<__EntryPoint>>
function main_array_combine_key_tostring() {
$foos = varray[new Foo];
$bar = array_combine($foos, $foos);

$arrays = varray[varray[1], 2];
$bar2 = array_combine($arrays, $arrays);

var_dump($bar);
var_dump($bar2);
echo "\n";
}

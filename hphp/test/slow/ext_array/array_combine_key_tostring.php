<?hh

class Foo { function __toString() { return 'Hello'; } }

<<__EntryPoint>>
function main_array_combine_key_tostring() {
$foos = varray[new Foo];
var_dump(array_combine($foos, $foos));
echo "\n";
}

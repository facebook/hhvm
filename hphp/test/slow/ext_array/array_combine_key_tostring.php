<?hh

class Foo { function __toString()[] :mixed{ return 'Hello'; } }

<<__EntryPoint>>
function main_array_combine_key_tostring() :mixed{
$foos = varray[new Foo];
var_dump(array_combine($foos, $foos));
echo "\n";
}

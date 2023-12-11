<?hh

<<__EntryPoint>>
function main_count_values() :mixed{
$array = vec[1, "hello", 1, "world", "hello"];
var_dump(array_count_values($array));
}

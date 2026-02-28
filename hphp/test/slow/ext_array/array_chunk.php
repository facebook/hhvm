<?hh


<<__EntryPoint>>
function main_array_chunk() :mixed{
$input_array = vec["a", "b", "c", "d", "e"];
var_dump(array_chunk($input_array, 2));
var_dump(array_chunk($input_array, 2, true));
}

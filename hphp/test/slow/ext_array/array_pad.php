<?hh


<<__EntryPoint>>
function main_array_pad() :mixed{
$input = vec[12, 10, 9];
var_dump(array_pad($input, 5, 0));
var_dump(array_pad($input, -7, -1));
var_dump(array_pad($input, 2, "noop"));
$a = dict["9" => "b"];
var_dump(array_pad($a, -3, "test"));
}

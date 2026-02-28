<?hh


<<__EntryPoint>>
function main_array_slice() :mixed{
$input = vec["a", "b", "c", "d", "e"];

var_dump(array_slice($input, 2));
var_dump(array_slice($input, 2, null));
var_dump(array_slice($input, -2, 1));
var_dump(array_slice($input, 0, 3));
var_dump(array_slice($input, 0, 5));

// note the differences in the array keys
var_dump(array_slice($input, 2, -1));
var_dump(array_slice($input, 2, -1, true));

var_dump(array_slice(vec["a", "b", "c"], 1, 2, true));
var_dump(array_slice(vec["a", "b", "c"], 1, 2, false));
$a = dict["a" => "g", 0 => "a", 1 => "b", 2 => "c"];
unset($a['a']);
var_dump(array_slice($a, 1, 2, true));
var_dump(array_slice($a, 1, 2, false));

$a = dict["a" => 123, 0 => "a", 1 => "b", 2 => "c"];
unset($a['a']);
var_dump(array_slice($a, 1, 2, true));
var_dump(array_slice($a, 1, 2, false));

var_dump(array_slice(vec[123, "b", "c"], 1, 2, true));

var_dump(array_slice(vec[123, "b", "c"], 1, 2, false));

var_dump(array_slice(dict[12 => 1, 6 => 2], 0, 2, true));
var_dump(array_slice(dict[12 => 1, 6 => 2], 0, 2, false));
}

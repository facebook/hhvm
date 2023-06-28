<?hh

<<__EntryPoint>>
function main_array_splice() :mixed{
$params = darray["a" => "aaa", 0 => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, darray[123 => "test"]);
var_dump($params);

$params = darray["a" => "aaa", 1 => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, darray[123 => "test"]);
var_dump($params);

$params = darray["a" => "aaa", "0" => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, darray[123 => "test"]);
var_dump($params);

$params = darray["a" => "aaa", "1" => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, darray[123 => "test"]);
var_dump($params);

$input = varray["red", "green", "blue", "yellow"];
array_splice(inout $input, 2);
var_dump($input);

$input = varray["red", "green", "blue", "yellow"];
array_splice(inout $input, 2, null);
var_dump($input, varray["red", "green"]);

$input = varray["red", "green", "blue", "yellow"];
array_splice(inout $input, 1, -1);
var_dump($input, varray["red", "yellow"]);

$input = varray["red", "green", "blue", "yellow"];
array_splice(inout $input, 1, 4, "orange");
var_dump($input);

$input = varray["red", "green", "blue", "yellow"];
array_splice(inout $input, -1, 1, varray["black", "maroon"]);
var_dump($input);

$input = varray["red", "green", "blue", "yellow"];
array_splice(inout $input, 3, 0, "purple");
var_dump($input);
}

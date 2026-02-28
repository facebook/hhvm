<?hh

<<__EntryPoint>>
function main_array_splice() :mixed{
$params = dict["a" => "aaa", 0 => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, dict[123 => "test"]);
var_dump($params);

$params = dict["a" => "aaa", 1 => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, dict[123 => "test"]);
var_dump($params);

$params = dict["a" => "aaa", "0" => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, dict[123 => "test"]);
var_dump($params);

$params = dict["a" => "aaa", "1" => "apple"];
unset($params['a']);
array_splice(inout $params, 0, 0, dict[123 => "test"]);
var_dump($params);

$input = vec["red", "green", "blue", "yellow"];
array_splice(inout $input, 2);
var_dump($input);

$input = vec["red", "green", "blue", "yellow"];
array_splice(inout $input, 2, null);
var_dump($input, vec["red", "green"]);

$input = vec["red", "green", "blue", "yellow"];
array_splice(inout $input, 1, -1);
var_dump($input, vec["red", "yellow"]);

$input = vec["red", "green", "blue", "yellow"];
array_splice(inout $input, 1, 4, "orange");
var_dump($input);

$input = vec["red", "green", "blue", "yellow"];
array_splice(inout $input, -1, 1, vec["black", "maroon"]);
var_dump($input);

$input = vec["red", "green", "blue", "yellow"];
array_splice(inout $input, 3, 0, "purple");
var_dump($input);
}

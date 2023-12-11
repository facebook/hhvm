<?hh


<<__EntryPoint>>
function main_to_array() :mixed{
$closure = function() {};
$closure_in_array = vec[$closure];

var_dump(is_array($closure_in_array));
var_dump(count($closure_in_array) === 1);
var_dump($closure_in_array[0] === $closure);
}

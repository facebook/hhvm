<?hh


<<__EntryPoint>>
function main_array_push() :mixed{
$input = vec["orange", "banana"];
$size = array_push(inout $input, "apple", "raspberry");
var_dump($input);
var_dump($size);
}

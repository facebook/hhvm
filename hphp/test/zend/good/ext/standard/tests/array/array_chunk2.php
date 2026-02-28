<?hh <<__EntryPoint>> function main(): void {
$input_array = vec['a', 'b', 'c', 'd', 'e'];
var_dump(array_chunk($input_array, 0));
var_dump(array_chunk($input_array, 0, true));
var_dump(array_chunk($input_array, 1));
var_dump(array_chunk($input_array, 1, true));
var_dump(array_chunk($input_array, 2));
var_dump(array_chunk($input_array, 2, true));
var_dump(array_chunk($input_array, 10));
var_dump(array_chunk($input_array, 10, true));
}

<?hh <<__EntryPoint>> function main(): void {
$array = varray['-000', ' 001', 1, ' 123', '+123'];
var_dump(array_count_values($array));
}

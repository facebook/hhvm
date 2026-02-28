<?hh <<__EntryPoint>> function main(): void {
$arr = vec[1, 2, 3, 4, 5, 6];

var_dump(array_slice($arr, 0, (float)2));
var_dump(array_slice($arr, 0, (int)2));
}

<?hh
<<__EntryPoint>> function main(): void {
var_dump(array_slice(range(1, 3), 0, NULL, true));
var_dump(array_slice(range(1, 3), 0, 0, true));
var_dump(array_slice(range(1, 3), 0, NULL));
var_dump(array_slice(range(1, 3), 0, 0));

var_dump(array_slice(range(1, 3), -1, 0));
var_dump(array_slice(range(1, 3), -1, 0, true));
var_dump(array_slice(range(1, 3), -1, NULL));
var_dump(array_slice(range(1, 3), -1, NULL, true));


$a = 'foo';
var_dump(array_slice(range(1, 3), 0, $a));
var_dump(array_slice(range(1, 3), 0, $a));
var_dump($a);
}

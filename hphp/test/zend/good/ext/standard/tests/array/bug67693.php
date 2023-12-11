<?hh
<<__EntryPoint>> function main(): void {
$array = dict[-1 => 0];

array_pop(inout $array);

array_push(inout $array, 0);
array_push(inout $array, 0);

var_dump($array);

echo"\nDone";
}

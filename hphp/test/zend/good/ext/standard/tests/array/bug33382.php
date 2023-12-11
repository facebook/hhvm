<?hh
<<__EntryPoint>> function main(): void {
$array = vec[1,2,3,4,5];

sort(inout $array);

var_dump(array_reverse($array));

echo "Done\n";
}

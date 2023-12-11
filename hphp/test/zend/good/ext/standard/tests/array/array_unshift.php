<?hh
<<__EntryPoint>> function main(): void {
$a = vec[];
$s = "";
var_dump(array_unshift(inout $a, $s));
var_dump($a);
var_dump(array_unshift(inout $s, $a));
var_dump($a);
var_dump(array_unshift(inout $a, $a));
var_dump($a);

echo "Done\n";
}

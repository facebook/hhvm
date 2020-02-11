<?hh
<<__EntryPoint>> function main(): void {
$a = varray[];
$a[] = $a;

var_dump($a);
var_dump(json_encode($a));

echo "Done\n";
}

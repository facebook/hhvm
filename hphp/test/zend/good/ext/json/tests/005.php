<?hh
<<__EntryPoint>> function main(): void {
$a = vec[];
$a[] = $a;

var_dump($a);
var_dump(json_encode($a));

echo "Done\n";
}

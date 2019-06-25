<?hh
<<__EntryPoint>> function main(): void {
$a = array();
$a[] = $a;

var_dump($a);
var_dump(json_encode($a));

echo "Done\n";
}

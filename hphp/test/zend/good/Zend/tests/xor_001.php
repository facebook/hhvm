<?hh
<<__EntryPoint>> function main(): void {
$a = varray[1,2,3];
$b = array();

$c = $a ^ $b;
var_dump($c);

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
$a = varray[1,2,3];
$b = varray[1,2];

$a = ~$b;
var_dump($a);

echo "Done\n";
}

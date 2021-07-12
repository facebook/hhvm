<?hh
<<__EntryPoint>> function main(): void {
$a = varray[1,2,3];
$b = varray[];

$c = (int)$a ^ (int)$b;
var_dump($c);

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
$a = vec[1,2,3];
$b = vec[];

$c = (int)$a ^ (int)$b;
var_dump($c);

echo "Done\n";
}

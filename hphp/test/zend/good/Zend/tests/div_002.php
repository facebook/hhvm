<?hh
<<__EntryPoint>> function main(): void {
$a = vec[1,2,3];
$b = vec[1];

$c = $a / $b;
var_dump($c);

echo "Done\n";
}

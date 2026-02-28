<?hh
<<__EntryPoint>> function main(): void {
$a = vec[1,2,3];
$b = vec[1,2];

$a = ~$b;
var_dump($a);

echo "Done\n";
}

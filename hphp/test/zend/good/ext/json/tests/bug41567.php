<?hh
<<__EntryPoint>> function main(): void {
$a = json_encode(123456789.12345);
var_dump(json_decode($a));

echo "Done\n";
}

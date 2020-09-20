<?hh
<<__EntryPoint>> function main(): void {
$str = "a b c d e";
var_dump(count(sscanf("a ",'%1$s')));

echo "Done\n";
}

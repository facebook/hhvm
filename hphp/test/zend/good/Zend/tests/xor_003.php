<?hh
<<__EntryPoint>> function main(): void {
$t = true;
$f = false;

var_dump((int)$t ^ (int)$f);
var_dump((int)$t ^ (int)$t);
var_dump((int)$f ^ (int)$f);

echo "Done\n";
}

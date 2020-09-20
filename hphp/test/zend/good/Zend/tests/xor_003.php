<?hh
<<__EntryPoint>> function main(): void {
$t = true;
$f = false;

var_dump($t ^ $f);
var_dump($t ^ $t);
var_dump($f ^ $f);

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
$a = vec[1,2,3];

$o = new stdClass;
$o->prop = "value";

$c = $o + $a;
var_dump($c);

echo "Done\n";
}

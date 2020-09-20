<?hh
<<__EntryPoint>> function main(): void {
$a = varray[1,2,3];

$o = new stdclass;
$o->prop = "value";

$c = $o + $a;
var_dump($c);

echo "Done\n";
}

<?hh
<<__EntryPoint>> function main(): void {
$i = 75636;
$s1 = "this is a string";
$s2 = "876222numeric";
$s3 = "48474874";
$s4 = "25.68";

$c = $i + $s1;
var_dump($c);

$c = $i + $s2;
var_dump($c);

$c = $i + $s3;
var_dump($c);

$c = $i + $s4;
var_dump($c);

$c = $s1 + $i;
var_dump($c);

$c = $s2 + $i;
var_dump($c);

$c = $s3 + $i;
var_dump($c);

$c = $s4 + $i;
var_dump($c);

echo "Done\n";
}

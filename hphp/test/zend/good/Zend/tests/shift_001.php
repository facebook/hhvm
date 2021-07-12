<?hh
<<__EntryPoint>> function main(): void {
$s = "123";
$s1 = "test";
$s2 = "45345some";

$s = (int)($s);

$s <<= 2;
var_dump($s);

$s1 = (int)($s1);

$s1 <<= 1;
var_dump($s1);

$s2 = (int)($s2);

$s2 <<= 3;
var_dump($s2);

echo "Done\n";
}

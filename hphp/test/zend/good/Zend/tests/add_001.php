<?hh
<<__EntryPoint>> function main(): void {
$a = varray[1,2,3];
$b = varray["str", "here"];

$c = $a + $b;
var_dump($c);

$a = varray[1,2,3];
$b = varray[1,2,4];

$c = $a + $b;
var_dump($c);

$a = darray["a" => "aaa", 0 => 2, 1 => 3];
$b = darray[0 => 1, 1 => 2, "a" => "bbbbbb"];

$c = $a + $b;
var_dump($c);

$a += $b;
var_dump($c);

$a += $a;
var_dump($c);

echo "Done\n";
}

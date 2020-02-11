<?hh
<<__EntryPoint>> function main(): void {
$a = varray[];
$b = 3;
$c = varray[5];
array_diff($a, $b, $c);
echo "OK!";
}

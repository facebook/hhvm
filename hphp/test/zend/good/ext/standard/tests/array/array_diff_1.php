<?hh
<<__EntryPoint>> function main(): void {
$a = vec[];
$b = 3;
$c = vec[5];
array_diff($a, $b, $c);
echo "OK!";
}

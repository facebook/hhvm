<?hh
<<__EntryPoint>> function main(): void {
$a = array();
$b = 3;
$c = array(5);
array_diff($a, $b, $c);
echo "OK!";
}

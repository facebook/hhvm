<?hh
<<__EntryPoint>> function main(): void {
$a = vec['a', 'b', 'c'];
$last = array_shift(inout $a);
$a[] = 'a';
var_dump($a);

$a = dict['a' => 1, 'b' => 2, 'c' => 3];
$last = array_shift(inout $a);
$a[] = 'a';
var_dump($a);
}

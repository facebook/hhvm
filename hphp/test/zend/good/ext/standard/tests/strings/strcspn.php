<?hh <<__EntryPoint>> function main(): void {
$a = "22222222aaaa bbb1111 cccc";
$b = "1234";
var_dump($a);
var_dump($b);
var_dump(strcspn($a,$b));
var_dump(strcspn($a,$b,9));
var_dump(strcspn($a,$b,9,6));
var_dump(strcspn('a', 'B', 1, 2147483647));
}

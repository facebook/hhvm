<?hh
<<__EntryPoint>> function main(): void {
$arr[1][2][3][4][5];

echo $arr[1][2][3][4][5];

$arr[1][2][3][4][5]->foo;

$arr = dict[1 => new stdClass()];
$arr[1]->foo = 1;
}

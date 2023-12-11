<?hh
<<__EntryPoint>> function main(): void {
$s1 = shape('a' => 1, 'b' => 2);
$s2 = dict['c' => 3, 'd' => 4];

var_dump($s1 === $s2);
var_dump($s1 ==  $s2);
var_dump($s1 !== $s2);
var_dump($s1 !=  $s2);

var_dump($s2 === $s1);
var_dump($s2 ==  $s1);
var_dump($s2 !== $s1);
var_dump($s2 !=  $s1);
}

<?hh


<<__EntryPoint>>
function main_42() :mixed{
$info = vec['coffee', 'brown', 'caffeine'];
$a = dict[];
list($a[0], $a[1], $a[2]) = $info;
var_dump($a);
}

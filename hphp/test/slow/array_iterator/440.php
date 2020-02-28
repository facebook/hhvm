<?hh


<<__EntryPoint>>
function main_440() {
$a = varray[1, 2, 3, 4, 5, 6];
while ($v = each(inout $a)) {
 if ($v[1] < 4) $a[] = $v[1] + $v[1];
 }
var_dump($a);
$a = varray[1, 2, 3, 4, 5, 6];
foreach ($a as $k => $v) {
 if ($v >= 4) $a = $v + $v;
 }
var_dump($a);
}

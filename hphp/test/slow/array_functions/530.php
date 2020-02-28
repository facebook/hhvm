<?hh

class A {
 }

<<__EntryPoint>>
function main_530() {
$o = new A;
$f = '10';
$o->$f = 100;
$a = (array)$o;
$v = 1;
$a[10] = $v;
$a[11] = varray[$v];
var_dump($a);
$b = darray[10 => 10];
var_dump(array_diff_key($a, $b));
var_dump(array_merge($a, $b));
var_dump(array_merge_recursive($a, $b));
var_dump(array_reverse($a));
var_dump(array_chunk($a, 2));
}

<?hh

class X {
}

<<__EntryPoint>>
function main_1549() :mixed{
$a = new X;
$q = vec[1,2,3];
$a->foo = $q;
for ($i = 0;
 $i < 10;
 $i++) $a->{
'x'.$i}
 = clone $a;
$a->bar = $q;
$s = serialize($a);
var_dump($s);
$A = unserialize($s);
var_dump($A);
$r = $A->bar;
var_dump(array_keys($r));
}

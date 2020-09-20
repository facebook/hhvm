<?hh
function ref(inout $a, $b) {
 echo "$a $b";
 }
function val($a, $b)  {
 echo "$a $b";
 }


<<__EntryPoint>>
function main_1098() {
$x = 0;
$foo0 = isset($g) ? "ref" : "val";
$foo1 = isset($g) ? "val" : "ref";
$foo0($x, $x = 5);
$foo1(inout $x, $x = 5);
}

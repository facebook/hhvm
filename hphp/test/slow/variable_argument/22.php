<?hh


<<__EntryPoint>>
function main_22() :mixed{
$ar1 = varray[10, 100, 100, 0];
 $ar2 = varray[1, 3, 2, 4];
array_multisort2(inout $ar1, inout $ar2);
 var_dump($ar1, $ar2);
}

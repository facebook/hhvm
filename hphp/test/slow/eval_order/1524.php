<?hh


<<__EntryPoint>>
function main_1524() :mixed{
$a = varray[1,2,3];
 $b = varray[4,5,6];
 $i = 1;
 $a[$i++] = $b[$i++];
 var_dump($a);
}

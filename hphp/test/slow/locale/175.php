<?hh


<<__EntryPoint>>
function main_175() :mixed{
$a = varray[1];
$b = varray[2];
$arr = varray[$b, $a];
print $arr[0][0];
asort(inout $arr, SORT_REGULAR);
 print $arr[0][0];
}

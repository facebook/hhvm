<?hh


<<__EntryPoint>>
function main_175() :mixed{
$a = vec[1];
$b = vec[2];
$arr = vec[$b, $a];
print $arr[0][0];
asort(inout $arr, SORT_REGULAR);
 print $arr[0][0];
}

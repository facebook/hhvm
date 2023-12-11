<?hh


<<__EntryPoint>>
function main_1524() :mixed{
$a = vec[1,2,3];
 $b = vec[4,5,6];
 $i = 1;
 $a[$i++] = $b[$i++];
 var_dump($a);
}

<?hh


<<__EntryPoint>>
function main_1524() :mixed{
$a = vec[1,2,3];
 $b = vec[4,5,6];
 $i = 1;
 $li = $i; $i++; $ri = $i; $i++;
 $a[$li] = $b[$ri];
 var_dump($a);
}

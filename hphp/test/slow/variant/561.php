<?hh


<<__EntryPoint>>
function main_561() {
$a=1;
$a='t';
 $b = $a;
 $b[10]= 'AB';
 var_dump($a);
 var_dump($b);
}

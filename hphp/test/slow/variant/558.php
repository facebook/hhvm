<?hh


<<__EntryPoint>>
function main_558() :mixed{
$a=1;
$a='t';
 $b = $a;
 $a[0] = 'AB';
 var_dump($a);
 var_dump($b);
}

<?hh


<<__EntryPoint>>
function main_40() :mixed{
$c = 'old';
 $rhs = vec[1,'test',3]; list($a,list($c),$b) = $rhs; var_dump($rhs);
var_dump($a);
 var_dump($b);
 var_dump($c);
}

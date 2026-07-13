<?hh


<<__EntryPoint>>
function main_39() :mixed{
$rhs = vec[1,vec[2],3]; list($a,list($c),$b) = $rhs; var_dump($rhs);
var_dump($a);
 var_dump($b);
 var_dump($c);
}

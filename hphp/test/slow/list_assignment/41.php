<?hh


<<__EntryPoint>>
function main_41() :mixed{
$rhs = vec[1,vec[2],3]; list($a,list(),$b) = $rhs; var_dump($rhs);
var_dump($a);
 var_dump($b);
}

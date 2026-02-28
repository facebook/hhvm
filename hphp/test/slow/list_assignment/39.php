<?hh


<<__EntryPoint>>
function main_39() :mixed{
var_dump(list($a,list($c),$b) = vec[1,vec[2],3]);
var_dump($a);
 var_dump($b);
 var_dump($c);
}

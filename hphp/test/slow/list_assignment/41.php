<?hh


<<__EntryPoint>>
function main_41() :mixed{
var_dump(list($a,list(),$b) = vec[1,vec[2],3]);
var_dump($a);
 var_dump($b);
}

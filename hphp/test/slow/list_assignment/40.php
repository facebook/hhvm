<?hh


<<__EntryPoint>>
function main_40() {
$c = 'old';
 var_dump(list($a,list($c),$b) = varray[1,'test',3]);
var_dump($a);
 var_dump($b);
 var_dump($c);
}

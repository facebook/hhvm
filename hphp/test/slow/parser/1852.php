<?hh


<<__EntryPoint>>
function main_1852() :mixed{
($a) = 1;
var_dump($a);
$b = vec[];
($b)[] = 2;
var_dump($b[0]);
$c = new stdClass;
($c)->prop = 3;
var_dump($c->prop);
}

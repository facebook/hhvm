<?hh


<<__EntryPoint>>
function main_443() {
$a = varray[1, 2, 3];
$o = new ArrayIterator($a);
var_dump($o->next());
var_dump($o->rewind());
}

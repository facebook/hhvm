<?hh


<<__EntryPoint>>
function main_443() {
$a = varray[1, 2, 3];
$o = new ArrayIterator($a);
var_dump($o->next());
var_dump($o->rewind());
try { var_dump($o->seek()); } catch (Exception $e) { var_dump($e->getMessage()); }
var_dump($o->asort());
var_dump($o->ksort());
var_dump($o->natsort());
var_dump($o->natcasesort());
}

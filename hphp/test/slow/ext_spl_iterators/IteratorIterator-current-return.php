<?hh


<<__EntryPoint>>
function main_iterator_iterator_current_return() {
$I = new IteratorIterator(new ArrayIterator(varray[1,2,3]));
$I->rewind();
var_dump($I->current());
$I->next();
var_dump($I->current());
$I->next();
var_dump($I->current());
$I->next();
var_dump($I->current());
}

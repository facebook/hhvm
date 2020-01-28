<?hh


<<__EntryPoint>>
function main_append_iterator_rewinds_children() {
$a = new AppendIterator();
$a1 = new ArrayIterator(varray[1,2,3]);
$a1->next();
$a->append($a1);

var_dump($a->current());
}

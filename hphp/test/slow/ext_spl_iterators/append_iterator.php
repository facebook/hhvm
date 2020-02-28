<?hh


/* This is test/zend/bad/ext/spl/tests/iterator_042.php, except that it doesn't
 * test some systemlib + typehint violation + set_error_handler interactions
 * that should be fixed separately */

<<__EntryPoint>>
function main_append_iterator() {
$it = new AppendIterator;

$it->append(new ArrayIterator(varray[1]));
$it->append(new ArrayIterator(varray[21, 22]));

var_dump($it->getArrayIterator());

$it->append(new ArrayIterator(varray[31, 32, 33]));

var_dump($it->getArrayIterator());

$idx = 0;

foreach($it as $k => $v)
{
  echo '===' . $idx++ . "===\n";
  var_dump($it->getIteratorIndex());
  var_dump($k);
  var_dump($v);
}
}

<?php

/* This is test/zend/bad/ext/spl/tests/iterator_042.php, except that it doesn't
 * test some systemlib + typehint violation + set_error_handler interactions
 * that should be fixed separately */

$it = new AppendIterator;

$it->append(new ArrayIterator(array(1)));
$it->append(new ArrayIterator(array(21, 22)));

var_dump($it->getArrayIterator());

$it->append(new ArrayIterator(array(31, 32, 33)));

var_dump($it->getArrayIterator());

$idx = 0;

foreach($it as $k => $v)
{
  echo '===' . $idx++ . "===\n";
  var_dump($it->getIteratorIndex());
  var_dump($k);
  var_dump($v);
}

<?php

$array = new ArrayIterator(array('a', array('b', 'c')));
$regex = new RegexIterator($array, '/Array/');

foreach ($regex as $match) {
  var_dump($match); // We should never get here
}

$rArrayIterator = new RecursiveArrayIterator(
                    array('test1', array('tet3', 'test4', 'test5'))
                  );
$rRegexIterator = new RecursiveRegexIterator($rArrayIterator, '/^test/',
                    RecursiveRegexIterator::ALL_MATCHES);

foreach ($rRegexIterator as $key1 => $value1) {
  if ($rRegexIterator->hasChildren()) {
    // print all children
    echo "Children: ";
    foreach ($rRegexIterator->getChildren() as $key => $value) {
      echo $value . " ";
    }
    echo "\n";
  } else {
    echo "No children\n";
  }
}

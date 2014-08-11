<?php
Interface ExtendsIterator extends Iterator {
}
Interface ExtendsIteratorAggregate extends IteratorAggregate {
}
Class IteratorImpl implements Iterator {
	public function next() {}
	public function key() {}
	public function rewind() {}
	public function current() {}
	public function valid() {}
}
Class IterarorAggregateImpl implements IteratorAggregate {
	public function getIterator() {}
}
Class ExtendsIteratorImpl extends IteratorImpl {
}
Class ExtendsIteratorAggregateImpl extends IterarorAggregateImpl {
}
Class A {
}

$classes = array('Traversable', 'Iterator', 'IteratorAggregate', 'ExtendsIterator', 'ExtendsIteratorAggregate', 
	  'IteratorImpl', 'IterarorAggregateImpl', 'ExtendsIteratorImpl', 'ExtendsIteratorAggregateImpl', 'A');

foreach($classes as $class) {
	$rc = new ReflectionClass($class);
	echo "Is $class iterable? ";
	var_dump($rc->isIterateable());
}

echo "\nTest invalid params:\n";
$rc = new ReflectionClass('IteratorImpl');
var_dump($rc->isIterateable(null));
var_dump($rc->isIterateable(null, null));
var_dump($rc->isIterateable(1));
var_dump($rc->isIterateable(1.5));
var_dump($rc->isIterateable(true));
var_dump($rc->isIterateable('X'));
var_dump($rc->isIterateable(null));

echo "\nTest static invocation:\n";
ReflectionClass::isIterateable();

?>

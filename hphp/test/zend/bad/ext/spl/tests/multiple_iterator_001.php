<?php

$iter1 = new ArrayIterator(array(1,2,3));
$iter2 = new ArrayIterator(array(1,2));
$iter3 = new ArrayIterator(array(new stdClass(),"string",3));

$m = new MultipleIterator();

echo "-- Default flags, no iterators --\n";
foreach($m as $value) {
	var_dump($value);
}
var_dump($m->current());

$m->attachIterator($iter1);
$m->attachIterator($iter2);
$m->attachIterator($iter3);

echo "-- Default flags, MultipleIterator::MIT_NEED_ALL | MultipleIterator::MIT_KEYS_NUMERIC --\n";

var_dump($m->getFlags() === (MultipleIterator::MIT_NEED_ALL | MultipleIterator::MIT_KEYS_NUMERIC));

foreach($m as $key => $value) {
	var_dump($key, $value);
}
try {
	$m->current();
} catch(RuntimeException $e) {
	echo "RuntimeException thrown: " . $e->getMessage() . "\n";
}
try {
	$m->key();
} catch(RuntimeException $e) {
	echo "RuntimeException thrown: " . $e->getMessage() . "\n";
}

echo "-- Flags = MultipleIterator::MIT_NEED_ANY | MultipleIterator::MIT_KEYS_NUMERIC --\n";

$m->setFlags(MultipleIterator::MIT_NEED_ANY | MultipleIterator::MIT_KEYS_NUMERIC);
var_dump($m->getFlags() === (MultipleIterator::MIT_NEED_ANY | MultipleIterator::MIT_KEYS_NUMERIC));

foreach($m as $key => $value) {
	var_dump($key, $value);
}

echo "-- Default flags, added element --\n";

$m->setFlags(MultipleIterator::MIT_NEED_ALL | MultipleIterator::MIT_KEYS_NUMERIC);

$iter2[] = 3;
foreach($m as $key => $value) {
	var_dump($key, $value);
}

echo "-- Flags |= MultipleIterator::MIT_KEYS_ASSOC, with iterator associated with NULL --\n";

$m->setFlags(MultipleIterator::MIT_NEED_ALL | MultipleIterator::MIT_KEYS_ASSOC);
$m->rewind();
try {
	$m->current();
} catch(InvalidArgumentException $e) {
	echo "InvalidArgumentException thrown: " . $e->getMessage() . "\n";
}

echo "-- Flags |= MultipleIterator::MIT_KEYS_ASSOC --\n";

$m->attachIterator($iter1, "iter1");
$m->attachIterator($iter2, b"iter2");
$m->attachIterator($iter3, 3);

foreach($m as $key => $value) {
	var_dump($key, $value);
}

echo "-- Associate with invalid value --\n";

try {
	$m->attachIterator($iter3, new stdClass());
} catch(InvalidArgumentException $e) {
	echo "InvalidArgumentException thrown: " . $e->getMessage() . "\n";
}

echo "-- Associate with duplicate value --\n";

try {
	$m->attachIterator($iter3, "iter1");
} catch(InvalidArgumentException $e) {
	echo "InvalidArgumentException thrown: " . $e->getMessage() . "\n";
}

echo "-- Count, contains, detach, count, contains, iterate --\n";

var_dump($m->countIterators());
var_dump($m->containsIterator($iter2));
var_dump($m->detachIterator($iter2));
var_dump($m->countIterators());
var_dump($m->containsIterator($iter2));
foreach($m as $key => $value) {
	var_dump($key, $value);
}

?>
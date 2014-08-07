<?php

$dll = new SplDoublyLinkedList();

$dll->push('Never');
$dll->push('gonna');
$dll->push('give');

// at pos 0, shift everything up
$dll->add(0, 'you');
// Should be the end
$dll->add(4, 'up');
// Somewhere in the middle
$dll->add(2, 'let');

try {
  // Key 12 is unaccessible
  $dll->add(12, 'down');
} catch (OutOfRangeException $e) {
  echo $e->getMessage(), PHP_EOL;
}


foreach($dll as $key=>$val) {
  echo $key . '=>' . $val, PHP_EOL;
}

echo "count(): " . $dll->count(), PHP_EOL;
echo "top(): " . $dll->top(), PHP_EOL;
echo "bottom(): " . $dll->bottom(), PHP_EOL;

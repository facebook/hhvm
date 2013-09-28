<?php

// Create an iterator to be limited
$fruits = new ArrayIterator(array(
  'apple',
  'banana',
  'cherry',
  'damson',
  'elderberry'
));

// Loop over first three fruits only
foreach (new LimitIterator($fruits, 0, 3) as $fruit) {
  var_dump($fruit);
}

echo "\n";

// Loop from third fruit until the end
// Note: offset starts from zero for apple
foreach (new LimitIterator($fruits, 2) as $fruit) {
  var_dump($fruit);
}


<?php
/* Array from Bug Request #64493 test script */
$rows = array(
  456 => array('id' => '3', 'title' => 'Foo', 'date' => '2013-03-25'),
  457 => array('id' => '5', 'title' => 'Bar', 'date' => '2012-05-20'),
);

echo "-- pass null as second parameter to get back all columns indexed by third parameter --\n";
var_dump(array_column($rows, null, 'id'));

echo "-- pass null as second parameter and bogus third param to get back zero-indexed array of all columns --\n";
var_dump(array_column($rows, null, 'foo'));

echo "-- pass null as second parameter and no third param to get back array_values(input) --\n";
var_dump(array_column($rows, null));

echo "Done\n";
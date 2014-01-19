<?php

function main() {
  $test = array(
    array(new ArrayObject, null),
    array(new ArrayObject, new ArrayObject),
    array(new ArrayObject(array(1)), new ArrayObject(array(2))),
    array(new ArrayObject(array(1)), new ArrayObject(array(1, 1))),
    array(new ArrayObject(array(1, 1)), new ArrayObject(array(1, 1))),
  );

  $a = new ArrayObject;
  $a->foo = 'foo';
  $b = new ArrayObject;
  $b->foo = 'foo';
  $test[] = array($a, $b);

  $a = new ArrayObject;
  $a->a = 'a';
  $b = new ArrayObject;
  $b->b = 'b';
  $test[] = array($a, $b);

  $a = new ArrayObject(array(2));
  $a->{1} = '1';
  $b = new ArrayObject(array(1));
  $b->{2} = '2';
  $test[] = array($a, $b);

  foreach ($test as $k => $row) {
    // http://www.php.net/manual/en/language.operators.comparison.php
    echo "Starting $k\n";
    var_dump($row[0] == $row[1]);
    var_dump($row[0] === $row[1]);
    var_dump($row[0] != $row[1]);
    var_dump($row[0] <> $row[1]);
    var_dump($row[0] !== $row[1]);
    var_dump($row[0] < $row[1]);
    var_dump($row[0] > $row[1]);
    var_dump($row[0] <= $row[1]);
    var_dump($row[0] >= $row[1]);
  }
}
main();

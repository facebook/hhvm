<?php

function test($a) {
  $it = new ArrayIterator($a);
  while ($it->valid()) {
    var_dump($it->key());
    var_dump($it->current());
    $it->next();
  }
}
test(array('a' => 'x',           false => 'y',           '1' => false,           null => 'z',           'c' => 'w'));

<?php

function favorite_fruit() {
  yield 'Tim' => 'apple';
  yield 'Bob' => 'pear';
}

foreach (favorite_fruit() as $person => $fruit) {
  var_dump($person);
  var_dump($fruit);
}

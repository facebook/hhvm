<?php
function main() {
  var_dump((clone (HH\Vector {1})->getIterator())->current());
  var_dump((clone (HH\ImmVector {2})->getIterator())->current());
  var_dump((clone (HH\Map {'a' => 3})->getIterator())->current());
  var_dump((clone (HH\ImmMap {'a' => 4})->getIterator())->current());
  var_dump((clone (HH\Set {5})->getIterator())->current());
  var_dump((clone (HH\ImmSet {6})->getIterator())->current());
  var_dump((clone (HH\Pair {7,8})->getIterator())->current());
}
main();

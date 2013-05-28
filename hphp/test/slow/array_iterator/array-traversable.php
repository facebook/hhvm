<?php

interface IFoo {
}
function trav(Traversable $x) {
  echo "Traversable " . $x . " \n";
  var_dump($x);
}
function ktrav(KeyedTraversable $x) {
  echo "KeyedTraversable " . $x . " \n";
  var_dump($x);
}
function ind(Indexish $x) {
  echo "Indexish " . $x . " \n";
  var_dump($x);
}
function ifoo(IFoo $x) {
  echo "IFoo " . $x . " \n";
  var_dump($x);
}
function main() {
  $arr = array();
  var_dump($arr instanceof Traversable);
  var_dump($arr instanceof KeyedTraversable);
  var_dump($arr instanceof Indexish);
  var_dump($arr instanceof IFoo);
  trav($arr);
  ktrav($arr);
  ind($arr);
  ifoo($arr);
}
main();


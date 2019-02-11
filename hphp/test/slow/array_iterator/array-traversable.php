<?hh

interface IFoo {
}
function trav(Traversable $x) {
  echo "Traversable ", $x, "\n";
  var_dump($x);
}
function ktrav(KeyedTraversable $x) {
  echo "KeyedTraversable ", $x, "\n";
  var_dump($x);
}
function cont(Container $x) {
  echo "Container ", $x, "\n";
  var_dump($x);
}
function kcont(KeyedContainer $x) {
  echo "KeyedContainer ", $x, "\n";
  var_dump($x);
}
function ifoo(IFoo $x) {
  echo "IFoo ", $x, "\n";
  var_dump($x);
}
function main() {
  $arr = array();
  var_dump($arr instanceof Traversable);
  var_dump($arr instanceof KeyedTraversable);
  var_dump($arr instanceof Container);
  var_dump($arr instanceof KeyedContainer);
  var_dump($arr instanceof IFoo);
  trav($arr);
  ktrav($arr);
  cont($arr);
  kcont($arr);
  ifoo($arr);
}


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_array_traversable() {
error_reporting(error_reporting() & ~E_NOTICE);
main();
}

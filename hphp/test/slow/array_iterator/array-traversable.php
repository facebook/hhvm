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
  $arr = varray[];
  var_dump($arr is Traversable);
  var_dump($arr is KeyedTraversable);
  var_dump($arr is Container);
  var_dump($arr is KeyedContainer);
  var_dump($arr is IFoo);
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

<?hh

interface IFoo {
}
function trav(\HH\Traversable $x) {
  echo "Traversable\n";
  var_dump($x);
}
function ktrav(\HH\KeyedTraversable $x) {
  echo "KeyedTraversable\n";
  var_dump($x);
}
function cont(\HH\Container $x) {
  echo "Container\n";
  var_dump($x);
}
function kcont(\HH\KeyedContainer $x) {
  echo "KeyedContainer\n";
  var_dump($x);
}
function ifoo(IFoo $x) {
  echo "IFoo\n";
  var_dump($x);
}
function main() {
  $arr = varray[];
  var_dump($arr is \HH\Traversable);
  var_dump($arr is \HH\KeyedTraversable);
  var_dump($arr is \HH\Container);
  var_dump($arr is \HH\KeyedContainer);
  var_dump($arr is IFoo);
  trav($arr);
  ktrav($arr);
  cont($arr);
  kcont($arr);
  ifoo($arr);
}

<<__EntryPoint>>
function main_array_traversable_2() {
main();
}

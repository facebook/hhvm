<?hh

interface IFoo {
}
function trav(\HH\Traversable $x) :mixed{
  echo "Traversable\n";
  var_dump($x);
}
function ktrav(\HH\KeyedTraversable $x) :mixed{
  echo "KeyedTraversable\n";
  var_dump($x);
}
function cont(\HH\Container $x) :mixed{
  echo "Container\n";
  var_dump($x);
}
function kcont(\HH\KeyedContainer $x) :mixed{
  echo "KeyedContainer\n";
  var_dump($x);
}
function ifoo(IFoo $x) :mixed{
  echo "IFoo\n";
  var_dump($x);
}
function main() :mixed{
  $arr = vec[];
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
function main_array_traversable_2() :mixed{
main();
}

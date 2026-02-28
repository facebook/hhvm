<?hh

interface IFoo {
}
function trav(Traversable $x) :mixed{
  echo "Traversable Array\n";
  var_dump($x);
}
function ktrav(KeyedTraversable $x) :mixed{
  echo "KeyedTraversable Array\n";
  var_dump($x);
}
function cont(Container $x) :mixed{
  echo "Container Array\n";
  var_dump($x);
}
function kcont(KeyedContainer $x) :mixed{
  echo "KeyedContainer Array\n";
  var_dump($x);
}
function ifoo(IFoo $x) :mixed{
  echo "IFoo Array\n";
  var_dump($x);
}
function main() :mixed{
  $arr = vec[];
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


<<__EntryPoint>>
function main_array_traversable() :mixed{
main();
}

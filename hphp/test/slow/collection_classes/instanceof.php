<?hh
function test($v, $m, $s) :mixed{
  if ($v is MutableVector) var_dump($v == 1);
  if ($m is MutableMap) var_dump($m == 1);
  if ($s is MutableSet) var_dump($s == 1);
}

<<__EntryPoint>>
function main_instanceof() :mixed{
$vector = Vector {};
$map = Map {};
$set = Set {};
$pair = Pair {null, null};
echo "==== Vector ====\n";
var_dump($vector is Container);
var_dump($vector is KeyedContainer);
var_dump($vector is Traversable);
var_dump($vector is KeyedTraversable);
var_dump($vector is ConstSetAccess);
var_dump($vector is ConstIndexAccess);
var_dump($vector is ConstMapAccess);
var_dump($vector is ConstVector);
var_dump($vector is ConstMap);
var_dump($vector is ConstSet);
echo "==== Map ====\n";
var_dump($map is Traversable);
var_dump($map is KeyedTraversable);
var_dump($map is Container);
var_dump($map is KeyedContainer);
var_dump($map is ConstSetAccess);
var_dump($map is ConstIndexAccess);
var_dump($map is ConstMapAccess);
var_dump($map is ConstVector);
var_dump($map is ConstMap);
var_dump($map is ConstSet);
echo "==== Set ====\n";
var_dump($set is Traversable);
var_dump($set is KeyedTraversable);
var_dump($set is Container);
var_dump($set is KeyedContainer);
var_dump($set is ConstSetAccess);
var_dump($set is ConstIndexAccess);
var_dump($set is ConstMapAccess);
var_dump($set is ConstVector);
var_dump($set is ConstMap);
var_dump($set is ConstSet);
echo "==== Pair ====\n";
var_dump($pair is Traversable);
var_dump($pair is KeyedTraversable);
var_dump($pair is Container);
var_dump($pair is KeyedContainer);
var_dump($pair is ConstSetAccess);
var_dump($pair is ConstIndexAccess);
var_dump($pair is ConstMapAccess);
var_dump($pair is ConstVector);
var_dump($pair is ConstMap);
var_dump($pair is ConstSet);
echo "==== Jit tests ====\n";
test($vector, $map, $set);
}

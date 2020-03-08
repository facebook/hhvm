<?hh // strict

namespace NS_foreach;

class C {
  public Vector<mixed> $pVect = Vector {-1, -2};
  public Map<string, float> $pMap = Map {'a' => -3.5};
  public Set<string> $pSet = Set {'a'};
  public Pair<int, string> $pPair = Pair {-10, "aaa"};
}

function main(): void {
  $c = new C();

  echo "=========== iterate over empty array =============\n";

  $array = array();
  foreach ($array as $a) {
    echo $a . "\n";
  }

  echo "=========== iterate over array of strings =============\n";

  $colors = array("red", "white", "blue");
  foreach ($colors as $color) {
    echo $color . "\n";
  }
//  echo $color . "\n";	// unlike PHP, this variable is no longer in scope

// access each element's value and its element number

  $colors = array("red", "white", "blue");
  foreach ($colors as $index => $color) {
    echo "Index: $index; Color: $color\n";
    var_dump($index);
  }
//  echo "Index: $index; Color: $color\n";	// unlike PHP, these variables are no longer in scope

  echo "=========== iterate over some vectors of string =============\n";

  $vect = Vector {};
  foreach ($vect as $key => $value) {
    echo "Key: $key ; Value: $value\n";
  }

  $vect = Vector {"red", "white", "blue"};
  foreach ($vect as $key => $value) {
    echo "Key: $key ; Value: $value\n";
  }

  $vect = ImmVector {"red", "white", "blue"};
  foreach ($vect as $key => $value) {
    echo "Key: $key ; Value: $value\n";
  }

  echo "=========== iterate over some maps of float =============\n";

  $map = Map {};
  foreach ($map as $key => $value) {
    echo "Key: $key ; Value: $value\n";
  }

  $map = Map {'a' => 1.234, 'xx' => 234.45, 'qqq' => -34.54};
  foreach ($map as $key => $value) {
    echo "Key: $key ; Value: $value\n";
  }

  $map = ImmMap {'a' => 1.234, 'xx' => 234.45, 'qqq' => -34.54};
  foreach ($map as $key => $value) {
    echo "Key: $key ; Value: $value\n";
  }

/*
// Sets are not KeyedTraversable, so can't iterate over them.

  echo "=========== iterate over some sets of string =============\n";

  $set = Set {"red", "white", "blue"};
  foreach ($set as $key => $value) {
    echo "Key: $key; Value: $value\n";
  }
*/

  echo "=========== iterate over a pair of strings =============\n";

  $pair = Pair {"red", "green"};
  foreach ($pair as $key => $value) {
//    echo "Key: $key; Value: $value\n";
    var_dump($key);
    var_dump($value);
  }

  echo "=========== Modify the local copy of an element's value =============\n";

  $colors = array("red", "white", "blue");
  foreach ($colors as $color) {
    echo $color . "<-->";
    $color = "black";
    echo $color . "\n";
  }
  var_dump($colors);

  echo "=========== Modify the the actual element itself =============\n";

  $colors = array("red", "white", "blue");
  // UNSAFE (can't really use a & in strict mode)
  foreach ($colors as & $color) {  // note the &
    echo $color . "<-->";
    $color = "black";
    echo $color . "\n";
  }
  var_dump($colors);

  echo "=========== nested iterators =============\n";

  $ary = array();
  $ary[0][0] = "abc";
  $ary[0][1] = "ij";
  $ary[1][0] = "mnop";
  $ary[1][1] = "xyz";

  foreach ($ary as $e1) {
    foreach ($e1 as $e2) {
      echo "  $e2";
    }
    echo "\n";
  }

  $a = array(array(10, 20), array(1.2, 4.5), array(true, "abc"));
  foreach ($a as $key => $value) {
    echo "------\n";
    var_dump($key);
    var_dump($value);
  }

  $vect = Vector{Vector {10}, Vector {1.2, 4.5, 9.2}, Vector {true, "abc"}};
  foreach ($vect as $key1 => $value1) {
    echo "Key: $key1\n";
    foreach ($value1 as $key2 => $value2) {
      echo "\tKey: $key2; Value: $value2\n";
    }
    echo "\n";
  }

  echo "=========== test use of list =============\n";

  $a = array(array(10,20), array(1.2, 4.5), array(true, "abc"));
  foreach ($a as $key => list($v1, $v2)) {
    echo "------\n";
    var_dump($key);
    var_dump($v1);
    var_dump($v2);
  }

  echo "=========== test use of a vector as recipient =============\n";

  $a = array(Vector{10}, Vector{1.2, 4.5, 9.2}, Vector {true, "abc"});
  foreach ($a as $key => $c->pVect) {
    echo "------\n";
    var_dump($key);
    var_dump($c->pVect);
  }

  echo "=========== test use of a map as recipient =============\n";

  $a = array(Map {'x' => 1.2, 'z' => 4.5, 'q' => 9.2}, Map { 'zz' => 22.1, 'cc' => -2.1 });
  foreach ($a as $key => $c->pMap) {
    echo "------\n";
    var_dump($key);
    var_dump($c->pMap);
  }

  echo "=========== test use of set as recipient =============\n";

  $a = array(Set {"red", "white", "blue"}, Set {"aa", "bb"});
  foreach ($a as $key => $c->pSet) {
    echo "------\n";
    var_dump($key);
    var_dump($c->pSet);
  }

  echo "=========== test use of Pair as recipient =============\n";

  $a = array(Pair { 2, "red" }, Pair { 9, "white" }, Pair { 1, "blue" });
  foreach ($a as $key => $c->pPair) {
    echo "------\n";
    var_dump($key);
    var_dump($c->pPair[0]);
    var_dump($c->pPair[1]);
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();

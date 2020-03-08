<?hh // strict

namespace NS_collections_operations;

class C {}

function main(int $idx): void {
//  $str = "Hello";
//  eval("echo \$str . \"\\n\";");			// intrinsic eval not recognized

//  exit ("bye bye!\n");
//  die ("bye bye!\n");				// intrinsic die not recognized

  echo "========================== Vector =======================\n";

//  $v1 = Vector {5, $idx, $idx * 3};
//  $v1 = Vector {0 = 5, 1 = $idx, 2 = $idx * 3};	// no explicit keys allowed
  $v1 = (Vector {})->addAll(array(5, 10, 15));	// grouping parens needed
  $v1[] = 20;
  $v1[0] = -5;

//  $v1[] += 17;	// throws InvalidOperationException;
			// Cannot use [] with collections for reading in an lvalue context
//  $v1[]++;	// ditto
//  ++$v1[];	// ditto
//  $v1[]--;	// ditto
//  --$v1[];	// ditto

  echo "\$v1[1] = " . $v1[1] . "\n";
//  echo "\$v1[100] = " . $v1[100] . "\n";		// throws OutOfBoundsException
  var_dump($v1);

//  var_dump(Vector {2, 4, 6}[1]);			// unexpected '[', expecting ')'
  var_dump((Vector {2, 4, 6})[1]);		// grouping parens needed

  echo "Vector \$v1 = >$v1<\n";
//  $res = print "Vector \$v1 = >$v1<\n";		// intrinsic print not recognized

//  echo "empty(Vector {}) = >" . empty(Vector {}) . "<\n";	// intrinsic empty not recognized

//  echo "isset(\$v1) is " . (isset($v1) ? "True" : "False") . "\n"; // told not to use isset
//  unset($v1);					// intrinsic unset not recognized

  $v = list($min, $max, $avg) = Vector {0, 100, 67};
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);

  $v1 = Vector {Vector {true, false}, Pair {10, 20}};
  $v2 = clone $v1;
  var_dump($v1);
  var_dump($v2);

  $v1 = Vector {5, 10};
  $v2 = Vector {5, 10};
  $v3 = Vector {10, 5};
  $v4 = Vector {5, 10, 15};

  echo "Vector ==  #1: " . ($v1 == $v1 ? 'T' : 'F') . "\n";
  echo "Vector ==  #2: " . ($v1 == $v2 ? 'T' : 'F') . "\n";
  echo "Vector ==  #3: " . ($v1 == $v3 ? 'T' : 'F') . "\n";
  echo "Vector ==  #4: " . ($v1 == $v4 ? 'T' : 'F') . "\n";

  echo "Vector === #1: " . ($v1 === $v1 ? 'T' : 'F') . "\n";
  echo "Vector === #2: " . ($v1 === $v2 ? 'T' : 'F') . "\n";
  echo "Vector === #3: " . ($v1 === $v3 ? 'T' : 'F') . "\n";
  echo "Vector === #4: " . ($v1 === $v4 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from Vector =======================\n";

  $v1 = Vector {};
  $v2 = Vector {5.2, 10.1, 15.3};

  $b1 = (bool)$v1;
  $b2 = (bool)$v2;
  var_dump($b1, $b2);

//  $i1 = (int)$v1;		// conversion not permitted
//  $i2 = (int)$v2;		// conversion not permitted
//  var_dump($i1, $i2);

//  $f1 = (float)$v1;	// conversion not permitted
//  $f2 = (float)$v2;	// conversion not permitted
//  var_dump($f1, $f2);

  $s1 = (string)$v1;	// uses __tostring
  $s2 = (string)$v2;	// uses __tostring
  var_dump($s1, $s2);

//  $a1 = (array)$v1;	// (array) cast forbidden in strict mode; arrays with
//				// unspecified key and value types are not allowed
//  $a2 = (array)$v2;
//  $a1 = (array<float>)$v1;	// can't cast to typed arrays either
//  $a2 = (array<float>)$v2;

//  $o1 = (object)$v1;		// object not supported
//  $o2 = (object)$v2;
//  var_dump($o1, $o2);

  echo "========================== ImmVector =======================\n";

  $iv1 = ImmVector {5, 10, 15};
//  $iv1[] = 20;					// mutation not allowed
//  $iv1[0] = -5;					// mutation not allowed
  echo "\$iv1[1] = " . $iv1[1] . "\n";
//  echo "\$iv1[100] = " . $iv1[100] . "\n";	// throws OutOfBoundsException
  var_dump($iv1);

  $v = list($min, $max, $avg) = ImmVector {0, 100, 67};
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  var_dump($v);

  $iv1 = ImmVector{5, 10};
  $iv2 = ImmVector{5, 10};
  $iv3 = ImmVector{10, 5};
  $iv4 = ImmVector{5, 10, 15};

  echo "ImmVector ==  #1: " . ($iv1 == $iv1 ? 'T' : 'F') . "\n";
  echo "ImmVector ==  #2: " . ($iv1 == $iv2 ? 'T' : 'F') . "\n";
  echo "ImmVector ==  #3: " . ($iv1 == $iv3 ? 'T' : 'F') . "\n";
  echo "ImmVector ==  #4: " . ($iv1 == $iv4 ? 'T' : 'F') . "\n";
  echo "ImmVector == Vector: " . (ImmVector {5, 10} == Vector {5, 10} ? 'T' : 'F') . "\n";

  echo "ImmVector === #1: " . ($iv1 === $iv1 ? 'T' : 'F') . "\n";
  echo "ImmVector === #2: " . ($iv1 === $iv2 ? 'T' : 'F') . "\n";
  echo "ImmVector === #3: " . ($iv1 === $iv3 ? 'T' : 'F') . "\n";
  echo "ImmVector === #4: " . ($iv1 === $iv4 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from ImmVector =======================\n";

  $iv1 = ImmVector {};
  $iv2 = ImmVector {5.2, 10.1, 15.3};

  $b1 = (bool)$iv1;
  $b2 = (bool)$iv2;
  var_dump($b1, $b2);

  $s1 = (string)$iv1;	// uses __tostring
  $s2 = (string)$iv2;	// uses __tostring
  var_dump($s1, $s2);

  echo "========================== Map =======================\n";

  $m1 = Map {'red' => -1, 'green' => -4, 'red' => 5, 'green' => 12};
  var_dump($m1);
  $m1['blue'] = 35;				// add an element with a new key
  $m1['red'] = 6;					// change the value of an existing element
//  $m1[] = 20;					// can only append a value of type Pair
  $m1[] = Pair {'black', 43};			// append value 43 with key 'black'
  $m1[] = Pair {'red', 123};			// replaces existing element's value with 123
//  $m1[true] = 99;					// throws InvalidArgumentException
  echo "\$m1['red'] = " . $m1['red'] . "\n";
//  echo "\$m1['white'] = " . $m1['white'] . "\n";	// throws OutOfBoundsException
  var_dump($m1);

  echo "Map \$m1 = >$m1<\n";

//  $m1 = Map {0 => 0, 1 => 100, 2 => 67};
//  	$v = list($min, $max, $avg) = $m1;		// can't use a Map here

  $m1 = Map{2 => 5.2, 'a' => 10.1};
  $m2 = Map{2 => 5.2, 'a' => 10.1};
  $m3 = Map{'a' => 10.1, 2 => 5.2};
  $m4 = Map{2 => 5.2, 'a' => 10.1, 1 => 12};

  echo "Map ==  #1: " . ($m1 == $m1 ? 'T' : 'F') . "\n";
  echo "Map ==  #2: " . ($m1 == $m2 ? 'T' : 'F') . "\n";
  echo "Map ==  #3: " . ($m1 == $m3 ? 'T' : 'F') . "\n";
  echo "Map ==  #4: " . ($m1 == $m4 ? 'T' : 'F') . "\n";

  echo "Map === #1: " . ($m1 === $m1 ? 'T' : 'F') . "\n";
  echo "Map === #2: " . ($m1 === $m2 ? 'T' : 'F') . "\n";
  echo "Map === #3: " . ($m1 === $m3 ? 'T' : 'F') . "\n";
  echo "Map === #4: " . ($m1 === $m4 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from Map =======================\n";

  $m1 = Map {};
  $m2 = Map {2 => 5.2, 'a' => 10.1, 'x' => 15.3};

  $b1 = (bool)$m1;
  $b2 = (bool)$m2;
  var_dump($b1, $b2);

  $s1 = (string)$m1;	// uses __tostring
  $s2 = (string)$m2;	// uses __tostring
  var_dump($s1, $s2);

  echo "========================== ImmMap =======================\n";

  $im1 = ImmMap {'red' => 5, 'green' => 12};
//  $im1['blue'] = 35;				// mutation not allowed
//  $im1['red'] = 6;				// mutation not allowed
//  $im1[] = Pair {'black', 43};			// mutation not allowed
  echo "\$im1['red'] = " . $im1['red'] . "\n";
//  echo "\$im1['white'] = " . $im1['white'] . "\n";// throws OutOfBoundsException
  var_dump($im1);

  $im1 = ImmMap{2 => 5.2, 'a' => 10.1};
  $im2 = ImmMap{2 => 5.2, 'a' => 10.1};
  $im3 = ImmMap{'a' => 10.1, 2 => 5.2, };
  $im4 = ImmMap{2 => 5.2, 'a' => 10.1, 1 => 12};

  echo "ImmMap ==  #1: " . ($im1 == $im1 ? 'T' : 'F') . "\n";
  echo "ImmMap ==  #2: " . ($im1 == $im2 ? 'T' : 'F') . "\n";
  echo "ImmMap ==  #3: " . ($im1 == $im3 ? 'T' : 'F') . "\n";
  echo "ImmMap ==  #4: " . ($im1 == $im4 ? 'T' : 'F') . "\n";
  echo "ImmMap == Map: " . (ImmMap{2 => 5.2, 'a' => 10.1} == Map{2 => 5.2, 'a' => 10.1} ? 'T' : 'F') . "\n";

  echo "ImmMap === #1: " . ($im1 === $im1 ? 'T' : 'F') . "\n";
  echo "ImmMap === #2: " . ($im1 === $im2 ? 'T' : 'F') . "\n";
  echo "ImmMap === #3: " . ($im1 === $im3 ? 'T' : 'F') . "\n";
  echo "ImmMap === #4: " . ($im1 === $im4 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from ImmMap =======================\n";

  $im1 = ImmMap {};
  $im2 = ImmMap {2 => 5.2, 'a' => 10.1, 'x' => 15.3};

  $b1 = (bool)$im1;
  $b2 = (bool)$im2;
  var_dump($b1, $b2);

  $s1 = (string)$im1;	// uses __tostring
  $s2 = (string)$im2;	// uses __tostring
  var_dump($s1, $s2);

  echo "========================== Set =======================\n";

  $s1 = Set {1, 1, 1, 5, 10, 1, 15, 1};		// duplicates are allowed, but are ignored
  $s1[] = 'red';
  $s1[] = 20;
  $s1[] = 5;					// attempt to add a duplicate is ignored
  $s1->add(10);					// attempt to add a duplicate is ignored
//  $s1[] = true;					// throws InvalidArgumentException
//  $s1[0] = -5;					// [index] not permitted
//  echo "\$s1[1] = " . $s1[1] . "\n";		// [index] not permitted
  var_dump($s1);

  echo "Set \$s1 = >$s1<\n";

//  $v = list($min, $max, $avg) = $s1;		// can't use a Map here

  $s1 = Set{10, 'red'};
  $s2 = Set{10, 'red'};
  $s3 = Set{'red', 10};
  $s4 = Set{10, 'red', 12};

  echo "Set ==  #1: " . ($s1 == $s1 ? 'T' : 'F') . "\n";
  echo "Set ==  #2: " . ($s1 == $s2 ? 'T' : 'F') . "\n";
  echo "Set ==  #3: " . ($s1 == $s3 ? 'T' : 'F') . "\n";
  echo "Set ==  #4: " . ($s1 == $s4 ? 'T' : 'F') . "\n";

  echo "Set === #1: " . ($s1 === $s1 ? 'T' : 'F') . "\n";
  echo "Set === #2: " . ($s1 === $s2 ? 'T' : 'F') . "\n";
  echo "Set === #3: " . ($s1 === $s3 ? 'T' : 'F') . "\n";
  echo "Set === #4: " . ($s1 === $s4 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from Set =======================\n";

  $s1 = Set {};
  $s2 = Set {10, 'red'};

  $b1 = (bool)$s1;
  $b2 = (bool)$s2;
  var_dump($b1, $b2);

  $s1 = (string)$s1;	// uses __tostring
  $s2 = (string)$s2;	// uses __tostring
  var_dump($s1, $s2);

  echo "========================== ImmSet =======================\n";

  $is1 = ImmSet {1, 1, 1, 5, 10, 1, 'red', 1};	// duplicates are allowed, but are ignored
// $is1[] = 'red';					// [index] not permitted
//  echo "\$is1[1] = " . $is1[1] . "\n";		// [index] not permitted
  var_dump($is1);

  $is1 = ImmSet{10, 'red'};
  $is2 = ImmSet{10, 'red'};
  $is3 = ImmSet{'red', 10};
  $is4 = ImmSet{10, 'red', 12};

  echo "ImmSet ==  #1: " . ($is1 == $is1 ? 'T' : 'F') . "\n";
  echo "ImmSet ==  #2: " . ($is1 == $is2 ? 'T' : 'F') . "\n";
  echo "ImmSet ==  #3: " . ($is1 == $is3 ? 'T' : 'F') . "\n";
  echo "ImmSet ==  #4: " . ($is1 == $is4 ? 'T' : 'F') . "\n";
  echo "ImmSet == Set: " . (ImmSet{10, 'red'} == Set{10, 'red'} ? 'T' : 'F') . "\n";

  echo "ImmSet === #1: " . ($is1 === $is1 ? 'T' : 'F') . "\n";
  echo "ImmSet === #2: " . ($is1 === $is2 ? 'T' : 'F') . "\n";
  echo "ImmSet === #3: " . ($is1 === $is3 ? 'T' : 'F') . "\n";
  echo "ImmSet === #4: " . ($is1 === $is4 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from ImmSet =======================\n";

  $is1 = ImmSet {};
  $is2 = ImmSet {10, 'red'};

  $b1 = (bool)$is1;
  $b2 = (bool)$is2;
  var_dump($b1, $b2);

  $is1 = (string)$is1;	// uses __tostring
  $is2 = (string)$is2;	// uses __tostring
  var_dump($is1, $is2);

  echo "========================== Pair =======================\n";

//  $p1 = new Pair();				// throws InvalidOperationException
//  $p1 = Pair {0 = 55, 1 = 'abc'};			// no explicit keys allowed
  $p1 = Pair {55, new C()};

//  $p1[0] = 14;					// throws InvalidOperationException
//  $p1[0]++;					// ditto
//  --$p1[0];					// ditto
//  $p1[0] *= 3;					// ditto

//  $p1[] = 20;					// mutation not allowed
  echo "\$p1[0] = " . $p1[0] . "\n";
//  echo "\$p1[true] = " . $p1[true] . "\n";	// Pair elements can only be accessed with an integer literal
//  echo "\$p1[100] = " . $p1[100] . "\n";		// Invalid index for this pair
//  echo "\$p1[$idx] = " . $p1[$idx] . "\n";	// Pair elements can only be accessed with an integer literal
  var_dump($p1);

  echo "Pair \$p1 = >$p1<\n";

  $p2 = Pair {15, 75};
  
  // UNSAFE (type check error - diff D3113087 out to fix)
  $v = list($min, $max) = $p2;
  echo "\$min: $min, \$max: $max\n";
  var_dump($v);

  $p1 = Pair{10, 'red'};
  $p2 = Pair{10, 'red'};
  $p3 = Pair{'red', 10};

  echo "Pair ==  #1: " . ($p1 == $p1 ? 'T' : 'F') . "\n";
  echo "Pair ==  #2: " . ($p1 == $p2 ? 'T' : 'F') . "\n";
  echo "Pair ==  #3: " . ($p1 == $p3 ? 'T' : 'F') . "\n";

  echo "Pair === #1: " . ($p1 === $p1 ? 'T' : 'F') . "\n";
  echo "Pair === #2: " . ($p1 === $p2 ? 'T' : 'F') . "\n";
  echo "Pair === #3: " . ($p1 === $p3 ? 'T' : 'F') . "\n";

  echo "========================== Conversions from Pair =======================\n";

  $v2 = Pair {5.2, 'red'};

  $b2 = (bool)$v2;
  var_dump($b2);

  $s2 = (string)$v2;	// uses __tostring
  var_dump($s2);
}

/* HH_FIXME[1002] call to main in strict*/
main(100);

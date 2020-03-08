<?hh // strict

namespace NS_intrinsics;

function main(): void {
  echo "--------- test with full and omitted LHS vars -------------\n";

  $v = list($min, $max, $avg) = Vector {0, 100, 67};
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);

//  list($min, , $avg) = array(0, 100, 67);		// missing arguments not allowed
//  list($min, , $avg) = Vector {0, 100, 67};	// missing arguments not allowed
//  echo "\$min: $min, , \$avg: $avg\n";

  list($min, $max, ) = Vector {0, 100, 67};
  echo "\$min: $min, \$max: $max,\n";

  list($min, $max) = Vector {0, 100, 67};
  echo "\$min: $min, \$max: $max\n";

  list($min) = Vector {0, 100, 67};
  echo "\$min: $min\n";

  $v = list($first, $second) = Pair {-5, 7};
  echo "\$first: $first, \$second: $second\n";
  print_r($v);

//  $v = list($first) = Pair {-15, 17};		// must extract both elements

  echo "--------- test with more elements than variables -------------\n";

  $v = list($min, $max, $avg) = Vector {0, 100, 67, 22, 33};
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);

//  $v = list($first, $second, $third) = Pair {-5, 7}; // must extract both elements only

//  echo "--------- test with fewer elements than variables -------------\n";
//
//  $v = list($min, $max, $avg) = Vector {100, 500};	// Undefined offset: 2

  echo "--------- test with element being an array -------------\n";

  $v = list($min, $max, $avg) = Vector {0, Vector {1, 7}, Vector {3, 6}};
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);

  $v = list($first, $second) = Pair {Vector {1, 7}, Vector {3, 6}};
  echo "\$first: $first, \$second: $second\n";
  print_r($v);

  echo "--------- test with target vars being array elements -------------\n";

  $a = array();		// Hack needs $a to exist, whereas PHP doesn't
  $v = list($a[0], $a[2], $a[4]) = Vector {0, 100, 67};
  print_r($a);
  print_r($v);

  $a = array();
  // UNSAFE (type check error - diff D3113087 out to fix)
  $v = list($a[0], $a[2]) = Pair {12, 24};
  print_r($a);
  print_r($v);

  echo "--------- test with no variables -------------\n";

  $v = list() = Vector {0, 100, 67};
  print_r($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();

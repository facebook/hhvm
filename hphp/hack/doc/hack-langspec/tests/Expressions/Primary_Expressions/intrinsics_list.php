<?hh // strict

namespace NS_intrinsics_list;

function main(): void {
  $min = -99;
  $max = -99;
  $avg = -99;

///*
  echo "--------- test with full and omitted LHS vars -------------\n";

  $v = list($min, $max, $avg) = array(0, 100, 67);
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);
//*/

/*
// cannot assign a map-like array to a list

  $v = list($min, $max, $avg) = array(2 => 67, 1 => 100, 0 => 0);
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);
*/

/*
// Can't omit arguments in a list, EXCEPT for the final one

  list($min, , $avg) = array(0, 100, 67);
  echo "\$min: $min, , \$avg: $avg\n";

  list(, $max, $avg) = array(0, 100, 67);
  echo ", \$max: $max, \$avg: $avg\n";

  list(, , $avg) = array(0, 100, 67);
  echo ", , \$avg: $avg\n";
*/

///*
  // UNSAFE (type error here - seems to work for collection like Vector but not array, maybe?)
  list($min, $max,) = array(10, 1100, 167);
  echo "\$min: $min, \$max: $max,\n";

  // UNSAFE (type error here - seems to work for collection like Vector but not array, maybe?)
  list($min, $max) = array(20, 2100, 267);
  echo "\$min: $min, \$max: $max\n";

//  list($min, , ) = array(0, 100, 67);
//  echo "\$min: $min, ,\n";

  list($min) = array(30, 3100, 367);
  echo "\$min: $min\n";
//*/

///*
  echo "--------- test with more array elements than variables -------------\n";

  $v = list($min, $max, $avg) = array(40, 4100, 467, 22, 33);
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);
//*/

/*
  echo "--------- test with fewer array elements than variables -------------\n";

  $v = list($min, $max, $avg) = array(-100, -500);	// Undefined offset: 2
  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);
*/

/*
// can't use nested lists

  echo "--------- test with nested lists -------------\n";

  $v = list($min, list($max, $avg)) = [0, [67, 99, 100], 33];
//  echo "\$min: $min, \$max: $max, \$avg: $avg\n";
  print_r($v);
*/

///*
  echo "--------- test with target vars being array elements -------------\n";

  $a = array();
  $v = list($a[0], $a[2], $a[4]) = array(50, 5100, 567);
  print_r($a);
  print_r($v);
//*/

///*
  echo "--------- test with no variables -------------\n";

  $v = list() = array(0, 100, 67);
  print_r($v);
//*/
}

/* HH_FIXME[1002] call to main in strict*/
main();

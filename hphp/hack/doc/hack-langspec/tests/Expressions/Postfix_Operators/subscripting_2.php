<?hh // strict

namespace NS_subscripting_2;

function main(): void {
  echo "====== vector-like array without index; simple assignment =========\n";

  $a = array(33, -11);
  var_dump($a[] = 991);		// creates $a[2]
  var_dump($a);
  echo "------\n";

  echo "====== array without index; compound assignment =========\n";

  $a = array(33, -11);
  var_dump($a[] += 991);		// creates $a[2]
  var_dump($a);
  echo "------\n";

  $a = array(33, -11);
  var_dump($a[] -= 991);		// creates $a[2]
  var_dump($a);
  echo "------\n";

  $a = array(33, -11);
  var_dump($a[] *= 991);		// creates $a[2]
  var_dump($a);

  echo "====== array without index; ++/-- =========\n";

  $a = array(33, -11);
  var_dump($a[]++);
  var_dump($a);
  echo "------\n";

  var_dump(++$a[]);
  var_dump($a);
  echo "------\n";

  var_dump(--$a[]);
  var_dump($a);

// Hack does not permit a map-like array to have an element appended using an empty []
//
//  echo "====== map-like array without index; simple assignment =========\n";
//
//  $a = array(3 => 33, -1 => -11);
//  var_dump($a[] = 991);		// creates $a[4]
//  var_dump($a);
//  echo "------\n";

}

/* HH_FIXME[1002] call to main in strict*/
main();

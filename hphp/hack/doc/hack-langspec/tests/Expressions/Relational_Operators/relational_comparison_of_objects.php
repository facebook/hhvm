<?hh // strict

namespace NS_relational_comparison_of_objects;

class C1 {}
class C2 {}
class C3 { public int $x = -1; }
class C4 { public int $y = -10; public int $x = -11; }

function main(): void {
  $c1a = new C1(); // var_dump($c1a);
  $c1b = new C1(); // var_dump($c1b);
  $c2 = new C2();  // var_dump($c2);

/*
  var_dump($c1a >= 10);	// different types; incompatible
  var_dump($c1a < 10.5);	// different types; incompatible
  var_dump($c1a > '12');	// different types; incompatible
  var_dump($c1a <= true);	// different types; incompatible
*/

//  echo "\n===== compare instances of different object types =====\n\n";

//  var_dump($c1a >= $c2);	// different types; incompatible
	
//  var_dump($c1a <= $c2);	// different types; incompatible
//  var_dump($c1a <  $c2);	// different types; incompatible

  echo "\n===== compare instances of the same (empty) object type =====\n\n";

  var_dump($c1a >= $c1b);	// bool(true)
  var_dump($c1a >  $c1b);	// bool(false)
  var_dump($c1a <= $c1b);	// bool(true)
  var_dump($c1a <  $c1b);	// bool(false)

  echo "\n===== compare instances of the same object type with same values =====\n\n";

  $c3a = new C3(); $c3a->x = 5; // var_dump($c3a);
  $c3b = new C3(); $c3b->x = 5; // var_dump($c3b);

  var_dump($c3a >= $c3b);	// bool(true)
  var_dump($c3a >  $c3b);	// bool(false)
  var_dump($c3a <= $c3b);	// bool(true)
  var_dump($c3a <  $c3b);	// bool(false)

  echo "\n===== compare instances of the same object type with diff values =====\n\n";

  $c3b->x = 7; // var_dump($c3a); var_dump($c3b);

  var_dump($c3a >= $c3b);	// bool(false)
  var_dump($c3a >  $c3b);	// bool(false)
  var_dump($c3a <= $c3b);	// bool(true)
  var_dump($c3a <  $c3b);	// bool(true)

  echo "\n===== compare instances of the same object type with a pair of diff values =====\n\n";

// comparison seems to be done in lexical order of property definition; swapping
// the order of y and x gives different relational op results

  $c4a = new C4(); $c4a->x = 3; $c4a->y = 6; // var_dump($c4a);
  $c4b = new C4(); $c4b->x = 5; $c4b->y = 2; // var_dump($c4b);

  var_dump($c4a >= $c4b);	// bool(true)
  var_dump($c4a >  $c4b);	// bool(true)
  var_dump($c4a <= $c4b);	// bool(false)
  var_dump($c4a <  $c4b);	// bool(false)
}

/* HH_FIXME[1002] call to main in strict*/
main();

<?hh // strict

namespace NS_tuples;

class C {
  private ?(int, (string, float)) $prop = null;
}

function f1((int, string) $p): void {
  var_dump($p);
}

function f2(): (bool, array<int>, float) {
  return tuple(true, array(99, 88, 77), 10.5);
}

function ft1((int, int) $p1): void {}
function ft2(): (int, int) { return tuple(10, 20); }

function main(?int $p1 = 123): void {
//  $t1 = tuple();	// must have at least 1 element
  $t1 = tuple(10);	// while can't declare a tuple type with only 1 element, can create a 1-element tuple

//  $t1 = tuple('a' => 5, 6 => 7);	// only (implied) int keys are allowed

//  $t1 = tuple(2 => 5, 6 => 7);	// can't specify index numbers

  $t2 = tuple(10, true, 2.3, 10, 'abc', null, $p1, array(2, $p1), Vector {}, new C());
  var_dump($t2);

//  foreach ($t2 => $key as $value) { }	//	can't iterate over a tuple

//  $i = 0;
//  echo "\$t2[0] = " . $t2[$i] . "\n";	// tuple index must be an integer literal

  echo "\$t2[0] = >" . $t2[0] . "<\n";
  echo "\$t2[4] = >" . $t2[4] . "<\n";
//  echo "\$t2[12] = >" . $t2[12] . "<\n";
//  echo "\$t2[-2] = >" . $t2[-2] . "<\n";
}

//main(null);
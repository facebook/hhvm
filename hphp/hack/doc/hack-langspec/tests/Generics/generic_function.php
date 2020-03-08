<?hh // strict

namespace NS_generic_function;

// ------------------------------------------

function maxVal<T>(T $p1, T $p2): T {
  return $p1 > $p2 ? $p1 : $p2;
}

// ------------------------------------------

/*
function swap<T>(T $p1, T $p2): void {
//  $temp = new T(); // no can do! Generics can only be used in type hints since they are erased at runtime.
}
*/

// ------------------------------------------

// Won't work, as $dest is passed by value, not by reference

function arrayCopy<T>(array<T> $dest, array<T> $source, int $count): void {
  for ($i = 0; $i < $count; ++$i) {
    $dest[$i] = $source[$i];
  }
}

// ------------------------------------------

class Box<T> {
  public T $value;

  public function __construct(T $v) {
    $this->value = $v;
  }
}

// works, but have swapped boxed values, not the original variables themselves

function swap<T>(Box<T> $a, Box<T> $b): void {
  $temp = $a->value;
  $a->value = $b->value;
  $b->value = $temp;
}

// ------------------------------------------

function main(): void {
  echo "=============== maxVal ==================\n\n";

  echo "maxVal(10, 20) = " . maxVal(10, 20) . "\n";
  echo "maxVal(20, 10) = " . maxVal(20, 10) . "\n";
  echo "maxVal(10, 10) = " . maxVal(10, 10) . "\n";

  echo "maxVal(15.6, -20.78) = " . maxVal(15.6, -20.78) . "\n";

  echo "maxVal('red', 'green') = " . maxVal('red', 'green') . "\n";

// all are accepted; hmm!

  $res = maxVal(10, 20.5);
  echo "maxVal(10, 20.5) = " . $res . "\n";
  var_dump($res);

  $res = maxVal(20.5, 10);
  echo "maxVal(20.5, 10) = " . $res . "\n";
  var_dump($res);

  $res = maxVal(0, true);
  echo "maxVal(0, true) = " . $res . "\n";
  var_dump($res);		// type is bool

  $res = maxVal(1, false);
  echo "maxVal(1, false) = " . $res . "\n";
  var_dump($res);		// type is int

  echo "\n=============== arrayCopy ==================\n\n";

  $dest = array(0);
  $source = array(10, 20, 30, 40);
  arrayCopy($dest, $source, 2);
  var_dump($dest);
}

/* HH_FIXME[1002] call to main in strict*/
main();

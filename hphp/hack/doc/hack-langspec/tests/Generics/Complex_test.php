<?hh // strict

namespace NS_Complex_test;

require_once 'Complex.php';

function main(): void {
  echo "================== Complex<float> ===================\n\n";

  $c1 = new \NS_Complex\Complex(10.5, 5.67);
  echo "\$c1 = " . $c1 . "\n";

  $c2 = new \NS_Complex\Complex(-10.98, -123.45);
  echo "\$c2 = " . $c2 . "\n";

  echo "\$c1 + \$c2 = " . \NS_Complex\Complex::add($c1, $c2) . "\n";
  echo "\$c1 - \$c2 = " . \NS_Complex\Complex::subtract($c1, $c2) . "\n";

  echo "\n================== Complex<int> ===================\n\n";

  $c3 = new \NS_Complex\Complex(5, 6);
  echo "\$c3 = " . $c3 . "\n";

  $c4 = new \NS_Complex\Complex(-10, -12);
  echo "\$c4 = " . $c4 . "\n";

  echo "\$c3 + \$c4 = " . \NS_Complex\Complex::add($c3, $c4) . "\n";
  echo "\$c3 - \$c4 = " . \NS_Complex\Complex::subtract($c3, $c4) . "\n";

  echo "\n================== Complex<num> ===================\n\n";

  $c5 = new \NS_Complex\Complex(9, 5.4);
  echo "\$c5 = " . $c5 . "\n";

  $c6 = new \NS_Complex\Complex(12.2, -20);
  echo "\$c6 = " . $c6 . "\n";

  echo "\$c5 + \$c6 = " . \NS_Complex\Complex::add($c5, $c6) . "\n";
  echo "\$c5 - \$c6 = " . \NS_Complex\Complex::subtract($c5, $c6) . "\n";

  var_dump($c5, $c6, \NS_Complex\Complex::add($c5, $c6), \NS_Complex\Complex::subtract($c5, $c6));
}

/* HH_FIXME[1002] call to main in strict*/
main();
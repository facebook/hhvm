<?hh
<<__EntryPoint>> function main(): void {
$lefts = varray[
  0,
  123,
  true,
  123.456,
  "non-numeric",
  "789",
  null,
];
$rights = $lefts;

foreach ($lefts as $left) {
  foreach ($rights as $right) {
    var_dump($left, $right);

    echo "  + ";
    $a = $left;
    var_dump($a += $right);
    var_dump($a);

    echo "  - ";
    $a = $left;
    var_dump($a -= $right);
    var_dump($a);

    echo "  * ";
    $a = $left;
    var_dump($a *= $right);
    var_dump($a);

    echo "  . ";
    $a = $left;
    var_dump($a .= (string)($right));
    var_dump($a);

    echo "  / ";
    $a = $left;
    try {
      var_dump($a /= $right);
      var_dump($a);
    } catch (DivisionByZeroException $e) {
      echo "\n", $e->getMessage(), "\n";
    }

    echo "  % ";
    $a = $left;
    try {
      var_dump($a %= $right);
      var_dump($a);
    } catch (DivisionByZeroException $e) {
      echo "\n", $e->getMessage(), "\n";
    }

    if (!($left is string && $right is string)) {
      $l = (int)$left;
      $right = (int)$right;
    } else {
      $l = $left;
    }

    echo "  & ";
    $a = $l;
    var_dump($a &= $right);
    var_dump($a);

    echo "  | ";
    $a = $l;
    var_dump($a |= $right);
    var_dump($a);

    echo "  ^ ";
    $a = $l;
    var_dump($a ^= $right);
    var_dump($a);

    $l = (int)$left;
    $right = (int)$right;

    echo "  << ";
    $a = $l;
    var_dump($a <<= $right);
    var_dump($a);

    echo "  >> ";
    $a = $l;
    var_dump($a >>= $right);
    var_dump($a);
  }
}
}

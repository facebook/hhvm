<?hh

<<__EntryPoint>> function main(): void {
  $minusEq = function($a, $b) {
    try {
      $c = $a;
      $c -= $b;
      printf("%s -= %s = %s\n",
             var_export($a, true),
             var_export($b, true),
             var_export($c, true));
    } catch (ArithmeticError $ex) {
      printf("exception for %s -= %s: %s\n",
             var_export($a, true),
             var_export($b, true),
             $ex->getMessage());
      return -1;
    }
  };

  $max = PHP_INT_MAX;
  $min = 1 << 63;

  $minusEq($max, -1);
  $minusEq($max, 1);
  $minusEq("$max", -1);
  $minusEq($max, '-1');
  $minusEq("$max", '-1');

  $minusEq($min, 1);
  $minusEq($min, -1);
  $minusEq("$min", 1);
  $minusEq($min, '1');
  $minusEq("$min", '1');
}

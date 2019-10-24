<?hh

<<__EntryPoint>> function main(): void {
  $mul = function($a, $b) {
    try {
      $c = $a * $b;
      printf("%s * %s = %s\n",
             var_export($a, true),
             var_export($b, true),
             var_export($c, true));
    } catch (ArithmeticError $ex) {
      printf("exception for %s * %s: %s\n",
             var_export($a, true),
             var_export($b, true),
             $ex->getMessage());
      return -1;
    }
  };

  $max = PHP_INT_MAX;
  $min = 1 << 63;

  $mul($max, 2);
  $mul($max, -2);
  $mul("$max", 2);
  $mul($max, '2');
  $mul("$max", '2');

  $mul($min, -2);
  $mul($min, 2);
  $mul("$min", -2);
  $mul($min, '-2');
  $mul("$min", '-2');
}

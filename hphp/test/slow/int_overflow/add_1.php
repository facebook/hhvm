<?hh

<<__EntryPoint>> function main(): void {
  $add = function($a, $b) {
    try {
      $c = $a + $b;
      printf("%s + %s = %s\n",
             var_export($a, true),
             var_export($b, true),
             var_export($c, true));
    } catch (ArithmeticError $ex) {
      printf("exception for %s + %s: %s\n",
             var_export($a, true),
             var_export($b, true),
             $ex->getMessage());
      return -1;
    }
  };

  $max = PHP_INT_MAX;
  $min = 1 << 63;

  $add($max, 1);
  $add($max, -1);
  $add("$max", 1);
  $add($max, '1');
  $add("$max", '1');

  $add($min, -1);
  $add($min, 1);
  $add("$min", -1);
  $add($min, '-1');
  $add("$min", '-1');
}

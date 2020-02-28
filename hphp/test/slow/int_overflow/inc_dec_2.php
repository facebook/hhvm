<?hh

class C {
  public $max = PHP_INT_MAX;
  public $min = 1 << 63;
}

function print_op(
  mixed $before,
  mixed $assigned,
  mixed $after,
  string $op,
  bool $is_pre
): void {
  if ($is_pre) {
    printf("%s(%s) = %s (%s)\n",
           $op,
           var_export($before, true),
           var_export($assigned, true),
           var_export($after, true));
  } else {
    printf("(%s)%s = %s (%s)\n",
           var_export($before, true),
           $op,
           var_export($assigned, true),
           var_export($after, true));
  }
}

function print_ex(
  mixed $before,
  string $op,
  bool $is_pre,
  ArithmeticError $err
): void {
  if ($is_pre) {
    printf("exception for %s(%s): %s\n",
           $op,
           var_export($before, true),
           $err->getMessage());
  } else {
    printf("exception for (%s)%s: %s\n",
           var_export($before, true),
           $op,
           $err->getMessage());
  }
}

<<__EntryPoint>> function main(): void {
  $preInc = function($a) {
    $b = $a;
    try {
      $c = ++$a;
      print_op($b, $c, $a, "++", true);
    } catch (ArithmeticError $ex) {
      print_ex($b, "++", true, $ex);
      return -1;
    }
  };
  $postInc = function($a) {
    $b = $a;
    try {
      $c = $a++;
      print_op($b, $c, $a, "++", false);
    } catch (ArithmeticError $ex) {
      print_ex($b, "++", false, $ex);
      return -1;
    }
  };
  $preDec = function($a) {
    $b = $a;
    try {
      $c = --$a;
      print_op($b, $c, $a, "--", true);
    } catch (ArithmeticError $ex) {
      print_ex($b, "--", true, $ex);
      return -1;
    }
  };
  $postDec = function($a) {
    $b = $a;
    try {
      $c = $a--;
      print_op($b, $c, $a, "--", false);
    } catch (ArithmeticError $ex) {
      print_ex($b, "--", false, $ex);
      return -1;
   }
  };

  $max = PHP_INT_MAX;
  $min = 1 << 63;

  $preInc($max);
  $preInc("$max");

  $postInc($max);
  $postInc("$max");

  $preDec($min);
  $preDec("$min");

  $postDec($min);
  $postDec("$min");

  $values = varray[$min, $max, -4, 0, 5, "12", 5.2, "1.5", "abc", "", null];

  print "array post inc\n";
  $array = $values;
  for ($i = 0; $i < count($array); ++$i) {
    $a = $array[$i];
    try {
      $b = $array[$i]++;
      print_op($a, $b, $array[$i], "++", false);
    } catch (ArithmeticError $ex) {
      print_ex($a, "++", false, $ex);
    }
  }

  print "array pre inc\n";
  $array = $values;
  for ($i = 0; $i < count($array); ++$i) {
    $a = $array[$i];
    try {
      $b = ++$array[$i];
      print_op($a, $b, $array[$i], "++", true);
    } catch (ArithmeticError $ex) {
      print_ex($a, "++", true, $ex);
    }
  }

  print "array post dec\n";
  $array = $values;
  for ($i = 0; $i < count($array); ++$i) {
    $a = $array[$i];
    try {
      $b = $array[$i]--;
      print_op($a, $b, $array[$i], "--", false);
    } catch (ArithmeticError $ex) {
      print_ex($a, "--", false, $ex);
    }
  }

  print "array pre dec\n";
  $array = $values;
  for ($i = 0; $i < count($array); ++$i) {
    $a = $array[$i];
    try {
      $b = --$array[$i];
      print_op($a, $b, $array[$i], "--", true);
    } catch (ArithmeticError $ex) {
      print_ex($a, "--", true, $ex);
    }
  }

  print "properties\n";
  $x = new C;
  $a = $x->max;
  try {
    $b = $x->max++;
    print "C->max++ = {$x->max} ($b)\n";
  } catch (ArithmeticError $ex) {
    printf("exception for C->max++: %s\n", $ex->getMessage());
  }

  $x = new C;
  $a = $x->max;
  try {
    $b = ++$x->max;
    print "++C->max = {$x->max} ($b)\n";
  } catch (ArithmeticError $ex) {
    printf("exception for ++C->max: %s\n", $ex->getMessage());
  }

  $x = new C;
  $a = $x->min;
  try {
    $b = $x->min--;
    print "C->min-- = {$x->min} ($b)\n";
  } catch (ArithmeticError $ex) {
    printf("exception for C->max--: %s\n", $ex->getMessage());
  }

  $x = new C;
  $a = $x->min;
  try {
    $b = --$x->min;
    print "--C->min = {$x->min} ($b)\n";
  } catch (ArithmeticError $ex) {
    printf("exception for --C->max: %s\n", $ex->getMessage());
  }

  $x = new C;
  $a = $x->max;
  try {
    $x->max += 1;
    print "C->max += 1 -> {$x->max}\n";
  } catch (ArithmeticError $ex) {
    printf("exception for C->max += 1: %s\n", $ex->getMessage());
  }

  $x = new C;
  $a = $x->min;
  try {
    $x->min -= 1;
    print "C->min -= 1 -> {$x->min}\n";
  } catch (ArithmeticError $ex) {
    printf("exception for C->max -= 1: %s\n", $ex->getMessage());
  }

  $x = new C;
  $a = $x->max;
  try {
    $x->max *= 2;
    print "C->max *= 2 -> {$x->max}\n";
  } catch (ArithmeticError $ex) {
    printf("exception for C->max *= 2: %s\n", $ex->getMessage());
  }
}

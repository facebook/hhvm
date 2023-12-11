<?hh

<<__EntryPoint>> function main(): void {
  $nan = acos(1.01);
  $values = vec[-0.0, 0.0, 0, 0.0001, $nan, 1.0];
  $cmps = vec[
     function($x, $y) { return $x < $y; },
     function($x, $y) { return $x <= $y; },
     function($x, $y) { return $x > $y; },
     function($x, $y) { return $x >= $y; },
     function($x, $y) { return HH\Lib\Legacy_FIXME\eq($x, $y); },
     function($x, $y) { return HH\Lib\Legacy_FIXME\neq($x, $y); },
     function($x, $y) { return $x === $y; },
     function($x, $y) { return $x !== $y; }];
  $cmpJmps = vec[
     function($x, $y) { return $x < $y ? true : false; },
     function($x, $y) { return $x <= $y ? true : false; },
     function($x, $y) { return $x > $y ? true : false; },
     function($x, $y) { return $x >= $y ? true : false; },
     function($x, $y) { return HH\Lib\Legacy_FIXME\eq($x, $y) ? true : false; },
     function($x, $y) { return HH\Lib\Legacy_FIXME\neq($x, $y) ? true : false; },
     function($x, $y) { return $x === $y ? true : false; },
     function($x, $y) { return $x !== $y ? true : false; }];
  $cmpNames = vec["<", "<=", ">", ">=", "==", "!=", "===", "!=="];

  foreach ($values as $x) {
    foreach ($values as $y) {
      $x__str = (string)($x);
      $y__str = (string)($y);
      echo " ---- $x__str $y__str ----\n";
      for ($i = 0; $i < sizeof($cmps); $i++) {
        $x__str = (string)($x);
        $y__str = (string)($y);
        var_dump("    $x__str $cmpNames[$i] $y__str:", (string)($cmps[$i]($x, $y)));
        if ($cmps[$i]($x, $y) !== $cmpJmps[$i]($x, $y)) {
          echo "jmps !== cmps, $cmpNames[$i]\n";
        }
      }
    }
  }
}

<?hh

function main() {
  $nan = acos(1.01);
  $values = array(-0.0, 0.0, 0, 0.0001, $nan, 1.0);
  $cmps = array(
     function($x, $y) { return $x < $y; },
     function($x, $y) { return $x <= $y; },
     function($x, $y) { return $x > $y; },
     function($x, $y) { return $x >= $y; },
     function($x, $y) { return $x == $y; },
     function($x, $y) { return $x != $y; },
     function($x, $y) { return $x === $y; },
     function($x, $y) { return $x !== $y; });
  $cmpJmps = array(
     function($x, $y) { return $x < $y ? true : false; },
     function($x, $y) { return $x <= $y ? true : false; },
     function($x, $y) { return $x > $y ? true : false; },
     function($x, $y) { return $x >= $y ? true : false; },
     function($x, $y) { return $x == $y ? true : false; },
     function($x, $y) { return $x != $y ? true : false; },
     function($x, $y) { return $x === $y ? true : false; },
     function($x, $y) { return $x !== $y ? true : false; });
  $cmpNames = array("<", "<=", ">", ">=", "==", "!=", "===", "!==");

  foreach ($values as $x) {
    foreach ($values as $y) {
      echo " ---- $x $y ----\n";
      for ($i = 0; $i < sizeof($cmps); $i++) {
        var_dump("    $x $cmpNames[$i] $y:", (string)($cmps[$i]($x, $y)));
        if ($cmps[$i]($x, $y) !== $cmpJmps[$i]($x, $y)) {
          echo "jmps !== cmps, $cmpNames[$i]\n";
        }
      }
    }
  }
}
main();

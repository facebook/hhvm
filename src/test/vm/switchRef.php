<?

/*
 * Check that the translator correctly adapts to callsites with variable
 * argument reffiness.
 */

// 100 pseudo-random bits generated offline.
$randBits = array(
0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 
1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 
1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 
0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1);

function byVal($a1, $a2) {
  echo "byVal ";
  return $a1 * $a2;
}

function byRef($a1, &$va2) {
  echo "byRef ";
  $va2 *= $a1;
  return $va2;
}

function main() {
  $funcs = array("byVal", "byRef");
  $b = 2;
  $c = 3;
  global $randBits;
  foreach($randBits as $idx => $bit) {
    $funcs[$bit]($b, $c);
    echo "$idx: $b, $c\n";
  }
}

main();


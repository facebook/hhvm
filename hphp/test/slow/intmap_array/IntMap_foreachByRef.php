<?hh

function main() {
  $a = miarray();
  foreach ($a as &$val) {  // For now we don't warn on empty array
    $val = 0;
  }

  $a = miarray();
  $a[0] = 0;
  foreach ($a as &$val) {  // Should warn
    $val = -1;
  }
  $a[] = "sup"; // already downgraded, anything goes
  var_dump($a);
}

main();

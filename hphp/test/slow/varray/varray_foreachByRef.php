<?hh

function main() {
  $a = varray();
  foreach ($a as &$val) {  // For now we don't warn on empty array
    $val = 0;
  }

  $a = varray();
  $a[0] = 0;
  foreach ($a as &$val) {  // Should warn
    $val = -1;
  }
  $a[123] = "sup"; // already downgraded, anything goes
  var_dump($a);
}

main();

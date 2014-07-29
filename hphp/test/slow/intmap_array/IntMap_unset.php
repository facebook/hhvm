<?php

function cow_unset($arr) {
  foreach ($arr as $key => $val) {
    if ($val) {
      unset($arr[$key]);
    } else {
      unset($arr['123']);
    }
  }
  return $arr;
}

function main() {
  $a = hphp_miarray();
  $a[1] = "moo";
  unset($a["1"]);

  $a = hphp_miarray();
  $a[1] = "moo";
  unset($a[1]);
  var_dump($a);

  $a = hphp_miarray();
  $a[1] = "moo";
  unset($a["foo"]);

  $a = hphp_miarray();
  $key = "1";
  $a[1] = "moo";
  unset($a[$key]);

  $a = hphp_miarray();
  $key = 1;
  $a[1] = "moo";
  unset($a[$key]);
  var_dump($a);

  $a = hphp_miarray();
  $key = "foo";
  $a[1] = "moo";
  unset($a[$key]);

  $a = hphp_miarray();
  $a[1] = true;
  $a[2] = false;
  $b = cow_unset($a);
  $a[] = "warning";
}

main();

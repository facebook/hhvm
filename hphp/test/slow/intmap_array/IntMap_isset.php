<?php

function cow_isset($arr) {
  $a = isset($arr['123']);
  $b = isset($arr['456']);
  if ($a || $b) {
    return true;
  } else {
    return false;
  }
}

function main() {
  $a = hphp_miarray();
  $b = isset($a["string"]);
  if (!$b) {
    $b = isset($a["string"]);
  } else {
    var_dump($b);
  }

  $a = hphp_miarray();
  $key = "stringKey";
  $b = isset($a[$key]);
  if (!$b) {
    $b = isset($a[$key]);
  } else {
    var_dump($b);
  }

  $a = hphp_miarray();
  $a[123] = 100;
  var_dump(cow_isset($a));
  $a[] = "warning";
}

main();

<?php

function f() { return array(); }

function populateArray($max) {
  $a = f();
  for ($i = 0; $i < $max; ++$i) {
    $a[$i] = $i * 9;
  }

  for ($i = 0; $i < $max; ++$i) {
    if ($a[$i] != $i * 9) {
      echo "mismatch: "; echo $i; echo ": ";
      echo $a[$i]; echo "; should be "; echo $i * 9;
      echo "\n";
    }
  }
}

function main() {
  $a = f();
  echo "writing: "; echo ($a[1] = 66); echo "\n";
  echo "reading: "; echo $a[1]; echo "\n";

  populateArray(1000);
}

main();

<?php

function getArray() {
  return [1, 2, 3];
}

$a = getArray();
ezc_array_set($a, 0, 2);
print implode(", ", $a) . "\n";
print implode(", ", getArray()) . "\n";

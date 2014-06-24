<?php

function getArray() {
  return array(1, 2, 3);
}

$a = getArray();
ezc_array_val_set($a, 0, 2);
print implode(", ", $a) . "\n";
print implode(", ", getArray()) . "\n";

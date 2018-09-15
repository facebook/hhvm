<?php

<<__EntryPoint>>
function main_extra_by_reference_nested_over_same_array_mutation_using_set() {
$a = [0, 1];
foreach ($a as &$x) {
  foreach ($a as &$y) {
    echo "$x - $y\n";
    $a[3] = 3;
  }
}
}

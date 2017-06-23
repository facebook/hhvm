<?php
/* See extra_by_reference_nested_over_same_array_mutation_mixed.php which shows
 * that this also works as expected in the RFC if the array starts out as mixed,
 * and extra_by_reference_nested_over_same_array_mutation_using_set.php which
 * shows that this works if we promote to mixed via a set instead of an unset.
 */
$a = [0, 1, 2, 3];
foreach ($a as &$x) {
  foreach ($a as &$y) {
    echo "$x - $y\n";
    if ($x == 0 && $y == 1) {
      unset($a[1]);
      unset($a[2]);
    }
  }
}

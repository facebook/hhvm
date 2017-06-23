<?php
/* This test case works correctly in hhvm, but breaks in both php5 and php7,
 * which both output 1, then 4, then stop without iterating over the third
 * array at all.
 * See also 08_by_reference_replace_array.php.
 */
$a = [1,2]; $b = [[3,4], [5,6]]; next($b[0]);
foreach($a as &$v) {echo "$v\n"; if (!empty($b)) $a = array_shift($b);}

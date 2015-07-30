<?php

$lastday = mktime(0, 0, 0, 3, 0, 2000);
var_dump(strftime("Last day in Feb 2000 is: %d", $lastday));

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}

/**
 * We are not supporting negative parameters
 * lastday = f_mktime(0, 0, 0, 4, -31, 2000);
 * VS(f_strftime("Last day in Feb 2000 is: %d", lastday),
 *    "Last day in Feb 2000 is: 29");
 */

VS(date("M-d-Y", mktime(0, 0, 0, 12, 32, 1997)), "Jan-01-1998");
VS(date("M-d-Y", mktime(0, 0, 0, 13, 1, 1997)),  "Jan-01-1998");
VS(date("M-d-Y", mktime(0, 0, 0, 1, 1, 1998)),   "Jan-01-1998");
VS(date("M-d-Y", mktime(0, 0, 0, 1, 1, 98)),     "Jan-01-1998");
VS(date("M-d-Y", mktime(0, 0, 0, 1, 1, 1900)),   "Jan-01-1900");
VS(date("M-d-Y", mktime(0, 0, 0, 1, 1, 2100)),   "Jan-01-2100");
VS(date("M-d-Y", mktime(0, 0, 0, 1, 1, 110)),    "Jan-01-0110");

VS(date("h", mktime(9)), "09");

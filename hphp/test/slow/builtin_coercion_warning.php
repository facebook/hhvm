<?hh
function test_strlen($x) { strlen($x); }
function test_intdiv($a, $b) { intdiv($a, $b); }
function test_strrev($x) { strrev($x); }


test_strlen(true);
test_intdiv(true, true);
test_strrev(null);
test_intdiv("10.0", "2.4");
test_intdiv(100.0, 2.0);

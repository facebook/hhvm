<?hh
function f(&$a) { $a++; print $a; }
function g($a) { $a++; print $a; }
$a = 'f';
if (0) $a(3);
$val4 = 4;
if (0) f(&$val4);
g(5);
print "\n";
$val3 = 3;
f(&$val3);

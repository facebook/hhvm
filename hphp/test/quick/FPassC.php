<?
function f(&$a) { $a++; print $a; }
function g($a) { $a++; print $a; }
$a = 'f';
if (0) $a(3);
if (0) f(4); 
g(5); 
print "\n";
f(3); 
?>

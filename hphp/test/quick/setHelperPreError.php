<?hh

print "Test begin\n";

function f(&$x) {
  var_dump($x);
}
# VGetM.
$x = 1;
f(&$x[0]);
var_dump($x);

print "Test end\n";

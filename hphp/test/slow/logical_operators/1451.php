<?hh

function foo() {
 echo "foo";
 }
$a = (false && foo());
$b = (true  || foo());
$c = ($c || true);
$d = ($d && false);
var_dump($a, $b, $c, $d);

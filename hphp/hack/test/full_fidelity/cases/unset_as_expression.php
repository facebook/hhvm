<?hh

$f = ($o) ==> unset($o->foo); // Bad

$x = $f->foo();
unset($x[0]); // Good

<?hh

$f = ($o) ==> unset($o->foo); // Bad

$x = $f->foo();
unset($x); // Good

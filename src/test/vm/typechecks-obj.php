<?

class C { }
class D extends C { }

function f(C $c = null) { var_dump($c);}

$c = new C();
$d = new D();
f($c);
f($d);
f(null);
f(new stdclass());
echo "not reached\n";

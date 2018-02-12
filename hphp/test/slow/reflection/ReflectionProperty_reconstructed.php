<?hh

class C { public $p; }
class D { private static $p2; }
$rp = new ReflectionProperty('C', 'p');
$rp->__construct('D', 'p2');
var_dump($rp->class, $rp->name, $rp->isPublic(), $rp->isStatic());

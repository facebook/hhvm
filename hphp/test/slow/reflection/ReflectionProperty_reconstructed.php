<?hh

class C { public $p; }
class D { private static $p2; }

<<__EntryPoint>>
function main_reflection_property_reconstructed() :mixed{
$rp = new ReflectionProperty('C', 'p');
$rp->__construct('D', 'p2');
var_dump($rp->class, $rp->name, $rp->isPublic(), $rp->isStatic());
}

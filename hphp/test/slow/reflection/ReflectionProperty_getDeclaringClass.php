<?hh

class C {
  protected $p;
}
class D extends C {}


<<__EntryPoint>>
function main_reflection_property_get_declaring_class() :mixed{
$rp = new ReflectionProperty('D', 'p');
var_dump($rp->class);
var_dump($rp->getDeclaringClass()->getName());
}

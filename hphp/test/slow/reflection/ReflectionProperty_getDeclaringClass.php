<?hh

class C {
  protected $p;
}
class D extends C {}

$rp = new ReflectionProperty('D', 'p');
var_dump($rp->class);
var_dump($rp->getDeclaringClass()->getName());

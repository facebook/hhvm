<?hh // strict

class C {
  public int $foo = 123;
}


<<__EntryPoint>>
function main_reflection_property_default_value() {
$x = new C;
$x->foo = 456;
$x->bar = 'baz';

$rp = new ReflectionProperty($x, 'foo');
var_dump($rp->isDefault()); // True. Although we have changed the
                            // value, the property was declared in the
                            // class.

$rpc = new ReflectionProperty('C', 'foo');
var_dump($rpc->isDefault());

$rpn = new ReflectionProperty($x, 'bar');
var_dump($rpn->isDefault());

var_dump($rp->getValue($x));
var_dump($rpc->getValue($x));
var_dump($rpn->getValue($x));
}

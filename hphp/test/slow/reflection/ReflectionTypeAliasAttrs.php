<?hh // strict

type MyTypeNoAttrs = int;

<<Attr1(1337)>>
type MyType = int;

<<Attr2, Attr3>>
newtype MyNewType = int;

$x = new ReflectionTypeAlias('MyTypeNoAttrs');
echo $x->__toString();
var_dump($x->getAttributes());

$x = new ReflectionTypeAlias('MyType');
echo $x->__toString();
var_dump($x->getAttributes());
var_dump($x->getAttribute('NonExistentAttr'));
var_dump($x->getAttribute('Attr1'));

$x = new ReflectionTypeAlias('MyNewType');
echo $x->__toString();
$attrs = $x->getAttributes();
ksort($attrs);
var_dump($attrs);

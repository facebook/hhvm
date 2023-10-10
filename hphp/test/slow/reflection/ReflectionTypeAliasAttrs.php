<?hh

type MyTypeNoAttrs = int;

<<Attr1(1337)>>
type MyType = int;

<<Attr2, Attr3>>
newtype MyNewType = int;


<<__EntryPoint>>
function main_reflection_type_alias_attrs() :mixed{
$x = new ReflectionTypeAlias('MyTypeNoAttrs');
echo $x->__toString();
var_dump($x->getAttributes());

$x = new ReflectionTypeAlias('MyType');
echo $x->__toString();
var_dump($x->getAttributes());
var_dump($x->getAttribute('NonExistentAttr'));
var_dump($x->hasAttribute('NonExistentAttr'));
var_dump($x->getAttribute('Attr1'));
var_dump($x->hasAttribute('Attr1'));

$x = new ReflectionTypeAlias('MyNewType');
echo $x->__toString();
$attrs = $x->getAttributes();
ksort(inout $attrs);
var_dump($attrs);
}

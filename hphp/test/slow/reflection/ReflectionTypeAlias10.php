<?hh

require_once 'ReflectionTypeAlias.inc';
type MyType2 = MyType;
newtype MyOpaqueType2 = MyOpaqueType;

$x = new ReflectionTypeAlias('MyType2');
var_dump($x->getFileName());

$x = new ReflectionTypeAlias('MyOpaqueType2');
var_dump($x->getFileName());

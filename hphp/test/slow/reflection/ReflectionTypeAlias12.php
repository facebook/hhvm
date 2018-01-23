<?hh

type MyType = Vector<int>;
newtype MyOpaqueType = (function (string, Set<int>): void);
require_once 'ReflectionTypeAlias.inc';

$x = new ReflectionTypeAlias('MyType');
var_dump($x->getFileName());

$x = new ReflectionTypeAlias('MyOpaqueType');
var_dump($x->getFileName());

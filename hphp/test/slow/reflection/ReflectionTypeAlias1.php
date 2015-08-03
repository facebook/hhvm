<?hh

type MyType = Vector<int>;
newtype MyOpaqueType = (function (string, Set<int>): void);

$x = new ReflectionTypeAlias('MyType');
echo $x->__toString();
var_dump($x->getResolvedTypeStructure());

$x = new ReflectionTypeAlias('MyOpaqueType');
echo $x->__toString();
var_dump($x->getResolvedTypeStructure());

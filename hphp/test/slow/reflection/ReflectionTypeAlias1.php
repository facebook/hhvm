<?hh

type MyType = Vector<int>;
newtype MyOpaqueType = (function (string, Set<int>): void);


<<__EntryPoint>>
function main_reflection_type_alias1() :mixed{
$x = new ReflectionTypeAlias('MyType');
echo $x->__toString();
var_dump($x->getResolvedTypeStructure());

$x = new ReflectionTypeAlias('MyOpaqueType');
echo $x->__toString();
var_dump($x->getResolvedTypeStructure());
}

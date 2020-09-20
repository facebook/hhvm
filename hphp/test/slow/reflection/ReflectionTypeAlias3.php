<?hh

type Matrix<T> = ?Vector<Vector<T>>;


<<__EntryPoint>>
function main_reflection_type_alias3() {
$x = new ReflectionTypeAlias('Matrix');
echo $x->__toString();
var_dump($x->getResolvedTypeStructure());
}

<?hh

type Matrix<T> = ?Vector<Vector<T>>;

$x = new ReflectionTypeAlias('Matrix');
echo $x->__toString();
var_dump($x->getResolvedTypeStructure());

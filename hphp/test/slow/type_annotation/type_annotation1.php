<?hh
class C {
  const type T = ?darray<int, <<__Soft>> bool>;
  const type U = Map<arraykey, Vector<varray<int>>>;
  const type V = (int, ?float, bool);
  const type W = (function (): void);
  const type X = (function (mixed, resource): AnyArray);
}


<<__EntryPoint>>
function main_type_annotation1() :mixed{
$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'U');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'V');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'W');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'X');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
}

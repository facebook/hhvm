<?hh

class C {
  const type T = ?shape('field1'=>?bool, 'field2'=>int, 'field3'=>arraykey);
  const type U = Set<shape('field1'=>string,)>;
  const type V = shape('array'=>?Vector<string>,
                       'shape'=>shape('HH\\float'=>float,
                                      'HH\\num'=>num),
                       'HH\\int'=>int,
                      );
  const type X = Map<int, shape('foo'=>AnyArray, 'bar'=>bool)>;
}


<<__EntryPoint>>
function main_type_annotation_shape1() :mixed{
$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'U');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'V');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());

$x = new ReflectionTypeConstant('C', 'X');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
}

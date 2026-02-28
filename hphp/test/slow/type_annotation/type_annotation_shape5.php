<?hh

class D {
  const int L = 123;
  const int N = 456;
  const string M = 'foo';
  const string O = 'bar';
}

class C {
  const type T = shape(
    D::O => shape(D::L => string, D::N => int),
    D::M => bool,
  );
}


<<__EntryPoint>>
function main_type_annotation_shape5() :mixed{
$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
}

<?hh

class C {
  const type T = D;
  const type U = C;
}

class D {
  const type T = C;
}


<<__EntryPoint>>
function main_type_annotation_classes2() :mixed{
var_dump(type_structure(C::class, 'T'));
var_dump(type_structure(C::class, 'U'));
var_dump(type_structure(D::class, 'T'));
}

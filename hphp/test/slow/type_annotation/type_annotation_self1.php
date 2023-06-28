<?hh

class Base {
  const type U = self::V;
  const type V = bool;
}

interface I {
  const type K = self::IV;
  const type IV = bool;
}

class Child extends Base implements I {
  const type V = int;
  const type self = float;
  const type W = self::self;
}


<<__EntryPoint>>
function main_type_annotation_self1() :mixed{
var_dump(type_structure(Base::class, 'U'));

var_dump(type_structure(Child::class, 'U'));
var_dump(type_structure(Child::class, 'K'));
var_dump(type_structure(Child::class, 'W'));

$x = new ReflectionTypeConstant(Child::class, 'W');
var_dump($x->getAssignedTypeText());
}

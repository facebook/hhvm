<?hh

type MyAlias1 = C;
type MyAlias2 = MyAlias1;
type MyAlias3 = MyAlias2;

class C {
  const type T = shape(
    'foo'=>Vector<MyAlias3>,
    'bar'=>MyAlias3::U,
  );
  const type U = bool;
}

$x = new ReflectionTypeConstant(C::class, 'T');
var_dump($x->getAssignedTypeText());

var_dump(type_structure(C::class, 'T'));
var_dump(type_structure(MyAlias1::class));
var_dump(type_structure(MyAlias2::class));
var_dump(type_structure(MyAlias3::class));

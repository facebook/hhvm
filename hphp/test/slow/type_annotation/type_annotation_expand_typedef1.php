<?hh // strict

newtype MyAlias<T> = Set<T>;
newtype MyAlias2<Tk, Tv> = (function (Tk): Map<Tk, Tv>);

$r = new ReflectionTypeAlias('MyAlias');
var_dump($r->getTypeStructure());
var_dump(type_structure(MyAlias::class));

$r = new ReflectionTypeAlias('MyAlias2');
var_dump($r->getTypeStructure());
var_dump(type_structure(MyAlias2::class));

class C {
  const type T = MyAlias2<int, MyAlias<string>>;
}

var_dump(type_structure(C::class, 'T'));

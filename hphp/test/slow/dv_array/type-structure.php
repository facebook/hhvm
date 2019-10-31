<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type T1 = (bool, int, string, array);
type T2 = shape('a' => bool, 'c' => int, 'd' => array);
type T3 = varray;
type T4 = darray;
type T5 = varray_or_darray;

class C {
  const type T = ?array<int, @bool>;
  const type U = Map<arraykey, Vector<array<int>>>;
  const type V = (int, ?float, bool);
  const type W = (function (): void);
  const type X = (function (mixed, resource): array);
}

newtype MyAlias<T> = Set<T>;
newtype MyAlias2<Tk, Tv> = (function (Tk): Map<Tk, Tv>);

class C2 {
  const type T = MyAlias2<int, MyAlias<string>>;
}

class C3 {
  const type T = int;
  const type U = C3::T;
  const type V = D::T;
  const type W = D;
}

class D {
  const type T = C3::U;
  const type U = C3::W::V;
  const type V = bool;
}

function dump($x) {
  var_dump(
    __hhvm_intrinsics\serialize_keep_dvarrays($x)
  );
}

function main() {
  dump(type_structure(T1::class));
  dump(type_structure(T2::class));
  dump(type_structure(T3::class));
  dump(type_structure(T4::class));
  dump(type_structure(T5::class));

  $x = new ReflectionTypeConstant('C', 'T');
  dump($x->getTypeStructure());
  $x = new ReflectionTypeConstant('C', 'U');
  dump($x->getTypeStructure());
  $x = new ReflectionTypeConstant('C', 'V');
  dump($x->getTypeStructure());
  $x = new ReflectionTypeConstant('C', 'W');
  dump($x->getTypeStructure());
  $x = new ReflectionTypeConstant('C', 'X');
  dump($x->getTypeStructure());

  $x = new ReflectionTypeAlias('MyAlias');
  dump($x->getTypeStructure());
  dump(type_structure(MyAlias::class));
  $x = new ReflectionTypeAlias('MyAlias2');
  dump($x->getTypeStructure());
  dump(type_structure(MyAlias2::class));

  dump(type_structure(C2::class, 'T'));

  dump(type_structure(C3::class, 'V'));
  dump(type_structure(D::class, 'U'));

}

<<__EntryPoint>>
function main_type_structure() {
main();
}

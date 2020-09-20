<?hh

type Foo = darray<string, mixed>;

abstract class C {
  <<__Reifiable>>
  abstract const type Ta;

  <<__Reifiable>>
  abstract const type Tb = vec<varray<int>>;

  const type Tc = this::Ta;

  abstract const type Td;

  <<__Reifiable>>
  const type Te = this::Td;
}

final class D extends C {
  const type Ta = (int, varray<int>);
  const type Td = int;
}

abstract class E {
  <<__Reifiable>>
  abstract const type Tb as Foo;
}

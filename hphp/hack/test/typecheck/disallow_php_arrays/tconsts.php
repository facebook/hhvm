<?hh

type Foo = darray<string, mixed>;

abstract class C {
  <<__DisallowPHPArrays>>
  abstract const type Ta;

  <<__DisallowPHPArrays>>
  abstract const type Tb = vec<varray<int>>;

  const type Tc = this::Ta;

  abstract const type Td;

  <<__DisallowPHPArrays>>
  const type Te = this::Td;
}

final class D extends C {
  const type Ta = (int, varray<int>);
  const type Td = int;
}

abstract class E {
  <<__DisallowPHPArrays>>
  abstract const type Tb as Foo;
}

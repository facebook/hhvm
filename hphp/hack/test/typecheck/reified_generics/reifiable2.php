<?hh

class C<reify T> {}

abstract class X {
  <<__Reifiable>>
  abstract const type TFoo;
}

abstract class Y extends X {
  const type X2 = C<this::TFoo>;
}

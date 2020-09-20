<?hh

class C<reify T> {}

abstract class X {
  <<__Reifiable>>
  abstract const type TFoo;
  abstract const type TBar;

  const type X2 = C<this::TFoo>;
  const type X3 = C<this::TBar>;
}

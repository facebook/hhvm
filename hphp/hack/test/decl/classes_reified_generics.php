<?hh

class C<reify T> {}

abstract class X {
  <<__Reifiable>>
  const type TFoo = int;

  const type X2 = C<this::TFoo>;
}

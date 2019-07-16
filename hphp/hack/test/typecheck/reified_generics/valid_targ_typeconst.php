<?hh // partial

class C<reify T> {}

class X {
  <<__Enforceable>>
  const type Enf = int;
  const type NotEnf = int;

  const type X2 = C<this::Enf>;
  const type X3 = C<this::NotEnf>;
}

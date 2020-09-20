<?hh // partial

class C<reify T> {}

class X {
  const type X1 = C<this>;
}

<?hh // partial

class Foo {
  const X = 'Y';
}

type Bar = Foo;

type S = shape(Bar::X => string);

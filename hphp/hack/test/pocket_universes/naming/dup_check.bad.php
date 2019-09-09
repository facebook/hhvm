<?hh // strict

class X {}
class Y {}

class A {
  enum F {
    case type T;
    case type T;

    case X x;
    case Y x;

    :@A( type T = int );

    :@A(
      type T = int,
      type T = int,

      x = 42,
      x = 42
    );
  }

  enum F {}
}

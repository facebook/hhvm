<?hh // strict

class Param<reify T> {
  public function __construct(public T $x) {}
}

class Foo {
  enum E {
    case type S;
    case Param<S> data;

    :@I (type S = int, data = new Param<int>(42));

  }
}

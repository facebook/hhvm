<?hh // strict

interface Foo {
  enum E {
    case string name;

    :@A (
      name = "A"
      );
  }
}

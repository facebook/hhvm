<?hh // strict

class C {
  enum E {
    case type T;
    :@A (type T = reify int);
  }
}

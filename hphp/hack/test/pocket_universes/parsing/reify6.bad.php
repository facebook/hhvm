<?hh // strict

class C {
  enum E {
    case type T;
    :@A (reify type T = int);
  }
}

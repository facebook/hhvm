<?hh

class C {
  enum E {
    case type T;
    case T val;
    :@I(type T = int, val = 42,);
  }
}

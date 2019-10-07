<?hh // strict

class C {}

class PU {
  enum E {
    case int v;
    case string s;
    case type T;
    case T val;

    :@X (
      type T = C,
      v = 1,
      s = "X",
      val = new C()
    );
    :@Y (
      type T = float,
      v = 2,
      s = "Y",
      val = 3.14
    );
  }
}

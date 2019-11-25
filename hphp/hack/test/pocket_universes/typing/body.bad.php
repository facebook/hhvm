<?hh // strict

class C {}

class CC {}

class PU {
  enum E {
    case int v;
    case string s;
    case type T;
    case T val;

    /* Unknown type D */
    :@X (
      type T = D,
      v = 1,
      s = "X",
      val = new D()
    );

    /* Incompatible class (using case type) */
    :@Y (
      type T = CC,
      v = 1,
      s = "X",
      val = new C()
    );

    /* Incompatible class (without case type) */
    :@Z (
      type T = float,
      v = 3,
      s = 4,
      val = 3.14
    );
  }
}

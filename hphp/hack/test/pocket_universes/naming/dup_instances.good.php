<?hh

class C {
  enum E {
    case type T;
    case T val;
    :@I(
      type T = int,
      val = 42
    );
  }
}

class D extends C {
  enum E {
    case string key;
    :@I(
      key = "I"
    );
  }
}

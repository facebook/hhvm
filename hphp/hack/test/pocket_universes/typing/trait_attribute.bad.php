<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case string name;
    :@A(name = "A");
  }
}

<<__Pu("E:name")>> // useless
class C0 extends C {
  enum E {
    :@B(name = "B");
  }
}

<<__Pu("E:foo")>> // unknown but useless since no traits
class C1 extends C {
  enum E {
    case int val;
    :@A(val = 42);
    :@B(name = "B", val = 1664);
  }
}

trait T {
  require extends C;
  enum E {
    :@T(name = "T");
  }
}

<<__Pu("E:name")>> // useless
class D0 extends C {
  use T;
  enum E {
    :@C(name = "C");
  }
}

<<__Pu("E:foo")>> // unknown
class D1 extends C {
  use T;
  enum E {
    :@C(name = "C");
  }
}

// missing
class D2 extends C {
  use T;
  enum E {}
}

// missing
class D3 extends C {
  use T;
}

<<__Pu("E:name,name")>> // duplicated field
class D4 extends C {
  use T;
}

<<__Pu("E:name", "E:name")>> // duplicated attribute
class D5 extends C {
  use T;
}

<<__Pu("E")>> // invalid
class D6 extends C {
  use T;
}

// missing
class E extends C {
  use T;
  enum E {
    case int val;
    :@A(val = 0);
    :@T(val = 42);
  }
}

trait TT {
  require extends C;
  enum E {
    :@TT(name = "TT");
  }
}

// missing
class F1 extends C {
  use T, TT;
}

// missing
class F2 extends C {
  use T, TT;
  enum E {}
}

// missing, works for traits too !
trait TX {
  use TT;
}

// multiple PU
class M {
  enum PU0 {
    case string msg;
    :@M(msg = "M");
  }
  enum PU1 {
    case string id;
    case int val;
    :@M(id = "PU1-M", val = 42);
  }
}

trait TM {
  require extends M;
  enum PU0 {
    :@TM(msg = "TM");
  }
}

class N extends M {
  use TM;
}

class P extends M {
  use TM;
  enum PU0 {
    :@P(msg = "P");
  }
}

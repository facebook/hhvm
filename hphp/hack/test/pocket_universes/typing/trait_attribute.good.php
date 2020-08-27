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

class C0 extends C {
  enum E {
    :@B(name = "B");
  }
}

class C1 extends C {
  enum E {
    case int val;
    :@A(val = 42);
    :@B(name = "B", val = 1664);
  }
}

class C2 extends C {
  enum E {}
}

class C3 extends C {
  enum E {}
}

trait T {
  require extends C;
  enum E {
    :@T(name = "T");
  }
}

class D0 extends C {
  use T;
  enum E {
    :@B(name = "B");
  }
}

class D1 extends C {
  use T;
  enum E {
    case int val;
    :@A(val = 42);
    :@B(name = "B", val = 1664);
    :@T(val = 314);
  }
}

<<__Pu("E:name")>>
class D2 extends C {
  use T;
  enum E {}
}

<<__Pu("E:name")>>
class D3 extends C {
  use T;
}

<<__Pu("E:name,")>>
class E extends C {
  use T;
  enum E {
    case int val;
    :@A(val = 0);
    :@T(val = 42);
  }
}

<<__Pu("E: name, ")>>
trait TT {
  use T;
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

<<__Pu("PU1:id, val", "PU0:msg")>>
class N extends M {
  use TM;
}

<<__Pu("PU1:id, val")>>
class P extends M {
  use TM;
  enum PU0 {
    :@P(msg = "P");
  }
  enum PU1 {
    case float f;
    :@M(f = 3.14);
  }
}

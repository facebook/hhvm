<?hh

<<__Pu("E:name")>>
class D2 extends C {
  use T;
  enum E {}
}

class C {
  enum E {
    case string name;
    :@A(name = "A");
  }
}

trait T {
  require extends C;
  enum E {
    :@T(name = "T");
  }
}

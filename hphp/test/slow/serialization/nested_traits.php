<?hh

trait tX {
  public $X = 'X';
}

trait tY {
  public $W = 'W';
}

trait tZ {
  public $Z = 'Z';
}

trait tA {
  use tX;
  use tY;
  use tZ;

  public $A = 'A';
}

trait tB {
  public $B = 'B';
}

class C {
  use tA;
  use tB;

  public $C = 'C';
}


<<__EntryPoint>>
function main_nested_traits() :mixed{
var_dump(serialize(new C()));
}

<?hh // strict

namespace NS_destructors;

class D1 {
  private function __destruct() {
    echo "In D1 destructor\n";
  }
}

class D2 extends D1 {
  public function __destruct() {
    echo "In D2 destructor\n";
//  parent::__destruct();		// can't access private destructor
  }
}

class D3 extends D2 {
  public function __destruct() {
    echo "In D3 destructor\n";
    parent::__destruct();
  }
}

class D4 extends D3 {
  public function __destruct() {
    echo "In D4 destructor\n";
    parent::__destruct();
  }
}

function main(): void {
//  $d1 = new D1();
//  $d2 = new D2();
//  $d3 = new D3();
  $d4 = new D4();
}

/* HH_FIXME[1002] call to main in strict*/
main();

<?hh // strict

namespace NS17 {
  const int CON1 = 100;

  function f(): void {
    echo "In " . __FUNCTION__ . "\n";
  }

  class C {
    const int C_CON = 200;
    public function f(): void {
      echo "In " . __NAMESPACE__ . "..." . __METHOD__ . "\n";
    }
  }

  interface I {
    const int I_CON = 300;
  }

  trait T {
    public function f(): void {
      echo "In " . __TRAIT__ . "..." . __NAMESPACE__ . "..." . __METHOD__ . "\n";
    }
  }
}

namespace NS18 {
  use \NS17\C, \NS17\I, \NS17\T;

  class D extends C implements I {
    use T;
  }

  function f(): void {
    $d = new D();
    var_dump($d);

    echo "CON1 = " . \NS17\CON1 . "\n";

    \NS17\f();

//  use \NS17\C as C2;
//  $c2 = new C2();
//  var_dump($c2);
  }
}

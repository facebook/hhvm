<?hh

trait T3 {
  public function bar3() :mixed{
    $x = () ==> {
      echo "bar3 lambda\n";
      $y = () ==> { echo "inner bar3 lambda\n"; };
      $y();
    };
    $x();
  }
}

trait T2 {
  use T3;
  use T3;

  public function bar2() :mixed{
    $x = () ==> {
      echo "bar2 lambda\n";
      $y = () ==> { echo "inner bar2 lambda\n"; };
      $y();
    };
    $x();
    $this->bar3();
  }
}

trait T {
  use T2;
  use T2;

  public function bar() :mixed{
    try {
      $x = () ==> {
        echo "bar lambda\n";
        $y = () ==> { echo "inner bar lambda\n"; };
        $y();
      };
      $x();
      $this->bar2();
    } finally {
      $z = () ==> {
        echo "finally lambda\n";
        $y = () ==> { echo "inner finally lambda\n"; };
        $y();
      };
      $z();
    }
  }
}

class A {
  use T;
  use T;

  public function foo() :mixed{
    $x = () ==> {
      echo "lambda1\n";
      $y = () ==> { echo "inner lambda1\n"; };
      $y();
    };
    $y = () ==> {
      echo "lambda2\n";
      $y = () ==> { echo "inner lambda2\n"; };
      $y();
    };
    $x();
    $y();
    $this->bar();
    $this->bar3();
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  $a->foo();
}

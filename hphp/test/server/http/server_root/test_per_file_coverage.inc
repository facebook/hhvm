<?hh

interface ITest {}

trait TTest {
  public function foo() : void {
    echo "TTest: foo!\n";
  }
}

class CTest {
  use TTest;
}

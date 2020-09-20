<?hh

trait THello1 {
  public function hello() {
    echo 'Hello';
  }
}

trait THello2 {
  public function hello() {
    echo 'Hello';
  }
}

class TraitsTest {
  use THello1;
  use THello2;
  public function hello() {
    echo 'Hello';
  }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->hello();
}

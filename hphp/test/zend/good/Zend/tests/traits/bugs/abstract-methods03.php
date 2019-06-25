<?hh

trait THello {
  public abstract function hello();
}

class TraitsTest {
  use THello;
  public function hello() {
    echo 'Hello';
  }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->hello();
}

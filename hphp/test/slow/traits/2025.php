<?hh

trait THello {
  public function hello() {
    echo 'Hello';
  }
}

class TraitsTest {
    use THello;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->hello();
}

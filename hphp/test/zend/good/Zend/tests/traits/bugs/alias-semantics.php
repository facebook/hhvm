<?hh

trait THello {
  public function a() {
    echo 'A';
  }
}

class TraitsTest {
    use THello { a as b; }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->a();
$test->b();
}

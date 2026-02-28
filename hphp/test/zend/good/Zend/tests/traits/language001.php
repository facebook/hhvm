<?hh

trait THello {
  public function hello() :mixed{
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

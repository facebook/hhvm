<?hh

trait THello {
  public abstract function hello():mixed;
}

trait THelloImpl {
  public function hello() :mixed{
    echo 'Hello';
  }
}

class TraitsTest {
    use THello;
    use THelloImpl;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->hello();
}

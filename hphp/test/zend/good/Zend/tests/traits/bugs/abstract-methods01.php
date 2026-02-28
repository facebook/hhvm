<?hh

trait THello {
  public abstract function hello():mixed;
}

class TraitsTest {
    use THello;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
$test->hello();
}

<?hh

trait THello {
  public abstract function hello():mixed;
}

trait THelloImpl {
  public function hello() :mixed{
    echo 'Hello';
  }
}

class TraitsTest1 {
    use THello;
    use THelloImpl;
}

class TraitsTest2 {
    use THelloImpl;
    use THello;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

$test = new TraitsTest1();
$test->hello();

$test = new TraitsTest2();
$test->hello();
}

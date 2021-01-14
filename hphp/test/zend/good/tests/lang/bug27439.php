<?hh

class test_props {
  public $a = 1;
  public $b = 2;
  public $c = 3;
}

class test {
  public $array = varray[1, 2, 3];
  public $string = "string";

  public function __construct() {
    $this->object = new test_props;
  }

  public function getArray() {
    return $this->array;
  }

  public function getString() {
    return $this->string;
  }

  public function case1() {
    foreach ($this->array as $foo) {
      echo $foo;
    }
    echo "\n";
  }

  public function case2() {
    try {
      foreach ($this->foobar as $foo)
        ;
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }

  public function case3() {
    try {
      foreach ($this->string as $foo)
        ;
    } catch (InvalidForeachArgumentException $e) {
      var_dump($e->getMessage());
    }
  }

  public function case4() {
    foreach ($this->getArray() as $foo)
      ;
  }

  public function case5() {
    try {
      foreach ($this->getString() as $foo)
        ;
    } catch (InvalidForeachArgumentException $e) {
      var_dump($e->getMessage());
    }
  }

  public function case6() {
    foreach ($this->object as $foo) {
      echo $foo;
    }
  }
}
<<__EntryPoint>>
function main(): void {
  $test = new test();
  $test->case1();
  $test->case2();
  $test->case3();
  $test->case4();
  $test->case5();
  $test->case6();
  echo "\n";
  echo "===DONE===";
}

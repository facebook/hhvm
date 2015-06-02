<?hh

class C {

  private function foo() {
    echo "in parent function\n";
    var_dump($this);
  }

  protected function bar() {
    echo 'in C::bar: ';
    echo 'get_called_class(): ' . get_called_class() . ', ';
    echo 'get_class($this): ' . get_class($this) . "\n\n";
    $this->foo();
  }
}

class D extends C {
  protected function bar() {
    echo $this;
    echo "in D::bar\n";
  }

  protected function direct() {
    echo "\ndirect:\n\n";
    array_map($x ==> parent::bar(), [1]); // capture $this
  }

  protected function nestedCapture() {
    echo "\nnestedCapture:\n\n";
    array_map($x ==> { // capture, because of inner capture
      (() ==> parent::bar())(); // capture
    }, [1]);
  }

  protected function nestedNoCapture() {
    echo "\nnestedNoCapture:\n\n";
    array_map($x ==> { // captures $this
      parent::bar();
      (() ==> static::bar())(); // doesn't capture $this
    }, [1]);
  }

  protected function reflectionInfo() {
    echo "\nreflectionInfo:\n\n";
    $l1 = () ==> var_dump($this);
    $l2 = () ==> parent::bar();
    $l3 = () ==> var_dump(1);

    $r1 = new ReflectionFunction($l1);
    $r2 = new ReflectionFunction($l2);
    $r3 = new ReflectionFunction($l3);

    var_dump($r1->getClosureThis());
    var_dump($r2->getClosureThis());
    var_dump($r3->getClosureThis());
  }

  public static function test() {
    (new D())->direct();
    (new D())->nestedCapture();
    (new D())->nestedNoCapture();
    (new D())->reflectionInfo();
  }
}

D::test();

echo "\nouter:\n\n";

$l4 = () ==> var_dump(2);
$r4 = new ReflectionFunction($l4);
var_dump($r4->getClosureThis());

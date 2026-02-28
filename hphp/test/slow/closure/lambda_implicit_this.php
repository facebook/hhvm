<?hh

class C {

  private function foo() :mixed{
    echo "in parent function\n";
    var_dump($this);
  }

  protected function bar() :mixed{
    echo 'in C::bar: ';
    echo 'static::class: ' . static::class . ', ';
    echo 'get_class($this): ' . get_class($this) . "\n\n";
    $this->foo();
  }
}

class D extends C {
  protected function bar() :mixed{
    echo $this;
    echo "in D::bar\n";
  }

  protected function direct() :mixed{
    echo "\ndirect:\n\n";
    array_map($x ==> parent::bar(), vec[1]); // capture $this
  }

  protected function nestedCapture() :mixed{
    echo "\nnestedCapture:\n\n";
    array_map($x ==> { // capture, because of inner capture
      (() ==> parent::bar())(); // capture
    }, vec[1]);
  }

  protected function reflectionInfo() :mixed{
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

  public static function test() :mixed{
    (new D())->direct();
    (new D())->nestedCapture();
    (new D())->reflectionInfo();
  }
}


<<__EntryPoint>>
function main_lambda_implicit_this() :mixed{
D::test();

echo "\nouter:\n\n";

$l4 = () ==> var_dump(2);
$r4 = new ReflectionFunction($l4);
var_dump($r4->getClosureThis());
}

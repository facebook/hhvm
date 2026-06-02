<?hh

class BestObject {
  <<__TestsBypassVisibility>>
  private function foo(): void { var_dump("foo"); }

  <<__TestsBypassVisibility>>
  protected function bar(): void { var_dump("bar"); }

  <<__TestsBypassVisibility>>
  private static function baz(): void { var_dump("baz"); }

  <<__TestsBypassVisibility>>
  protected static function qux(): void { var_dump("qux"); }

  <<__TestsBypassVisibility>>
  private int $priv_prop = 1;

  <<__TestsBypassVisibility>>
  protected int $prot_prop = 2;

  <<__TestsBypassVisibility>>
  private static int $spriv_prop = 3;

  <<__TestsBypassVisibility>>
  protected static int $sprot_prop = 4;
}

class WWWTest {}

class FooTest extends WWWTest {
  public function test(): void {
    $obj = new BestObject();
    $obj->foo();
    $obj->bar();
    BestObject::baz();
    BestObject::qux();
    var_dump($obj->priv_prop);
    var_dump($obj->prot_prop);
    var_dump(BestObject::$spriv_prop);
    var_dump(BestObject::$sprot_prop);
  }
}

class BarTest extends FooTest {
  public function test(): void {
    $obj = new BestObject();
    $obj->foo();
    $obj->bar();
    BestObject::baz();
    BestObject::qux();
    var_dump($obj->priv_prop);
    var_dump($obj->prot_prop);
    var_dump(BestObject::$spriv_prop);
    var_dump(BestObject::$sprot_prop);
  }
}

<<__EntryPoint>>
function main(): void {
  (new FooTest())->test();
  (new BarTest())->test();
}

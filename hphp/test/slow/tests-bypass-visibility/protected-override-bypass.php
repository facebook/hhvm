<?hh

class WWWTest {}

class Base {
  <<__TestsBypassVisibility>>
  protected function compute(): mixed { return "Base::compute"; }
}

class GoodChild extends Base {
  <<__Override, __TestsBypassVisibility>>
  protected function compute(): mixed { return "GoodChild::compute"; }
}

class OverrideTest extends WWWTest {
  public function test(): void {
    $base = new Base();
    echo $base->compute() . "\n";

    $child = new GoodChild();
    echo $child->compute() . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  (new OverrideTest())->test();
}

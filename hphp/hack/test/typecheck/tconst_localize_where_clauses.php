<?hh // strict
abstract class Box {
 abstract const type TResult;
 abstract public function getValue(): this::TResult;
}

class Foo2 {
  public function genx<TBox as Box, T>(
    TBox $box,
  ): T where T=TBox::TResult {
    return $box->getValue();
  }
}

class Bar2 extends Foo2 {

}

trait Foo1 {
  public function genx<TBox as Box, T>(
    TBox $box,
  ): T where T=TBox::TResult {
    return $box->getValue();
  }
}

class Bar1 {
  use Foo1;
}

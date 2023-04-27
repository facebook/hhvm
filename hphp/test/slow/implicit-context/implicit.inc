<?hh

class Base implements HH\IMemoizeParam {
  public function getInstanceKey()[]: string {
    return 'KEY' . $this->name();
  }
  public function name()[]: string { return static::class; }
}

class A extends Base {}
class B extends Base {}
class C extends Base {}
class D extends Base {}

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function start(int $context, (function (): int) $f)[zoned] {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: ?int {
    return parent::get();
  }
}

trait ClassContextDefs {
  const type T = Base;
  public static function start(Base $context, (function (): int) $f)[zoned, ctx $f] {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: Base {
    return parent::get() as nonnull;
  }
}

abstract final class ClassContext extends HH\ImplicitContext {
  use ClassContextDefs;
}

abstract final class ClassContext2 extends HH\ImplicitContext {
  use ClassContextDefs;
}

<?hh

class Base implements HH\IMemoizeParam {
  public function getInstanceKey(): string {
    return 'class ' . $this->name();
  }
  public function name(): string { return static::class; }
}

class A extends Base {}
class B extends Base {}
class C extends Base {}
class D extends Base {}

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function start(int $context, (function (): int) $f) {
    return parent::set($context, $f);
  }
  public static function getContext(): ?int {
    return parent::get();
  }
}

abstract final class ClassContext extends HH\ImplicitContext {
  const type T = Base;
  public static function start(Base $context, (function (): int) $f) {
    return parent::set($context, $f);
  }
  public static function getContext(): Base {
    return parent::get() as nonnull;
  }
}

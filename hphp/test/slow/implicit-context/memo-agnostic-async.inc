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


abstract final class TestAsyncContext extends HH\ImplicitContext {
  const type T = mixed;
  const bool IS_MEMO_SENSITIVE = false;
  public static async function genRunWith(mixed $context, (function (): mixed) $f) {
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext(): mixed {
    return parent::exists() ? parent::get() : null;
  }
}

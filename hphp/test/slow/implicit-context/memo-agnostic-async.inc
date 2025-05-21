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


abstract final class TestAsyncContext extends HH\HHVMTestMemoAgnosticImplicitContext {
  const type TData = mixed;
  const ctx CRun = [leak_safe];
  public static async function genRunWith(mixed $context, (function (): mixed) $f) {
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext(): mixed {
    return parent::exists() ? parent::get() : null;
  }
}

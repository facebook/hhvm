<?hh

class Base implements HH\IMemoizeParam {
  public function getInstanceKey()[]: string {
    return 'class ' . $this->name();
  }
  public function name()[]: string { return static::class; }
}

class A extends Base {}
class B extends Base {}
class C extends Base {}
class D extends Base {}

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  const bool IS_MEMO_SENSITIVE = true;
  const ctx CRun = [zoned];
  public static async function genStart(int $context, (function (): int) $f)[zoned] {
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: ?int {
    return parent::get();
  }
}

abstract final class ClassContext extends HH\ImplicitContext {
  const type T = Base;
  const bool IS_MEMO_SENSITIVE = true;
  const ctx CRun = [zoned];
  public static async function genStart(Base $context, (function (): int) $f)[zoned] {
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: ?Base {
    return parent::get();
  }
}

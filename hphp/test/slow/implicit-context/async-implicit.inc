<?hh

class Base implements HH\IPureMemoizeParam {
  public int $x;
  public function __construct(int $x = 0) { $this->x = $x; }
  public function getPayload()[zoned]: int { return $this->x; }
  public function getInstanceKey()[]: string {
    return 'KEY' . $this->name();
  }
  public function name()[]: string { return static::class; }
}

class A extends Base {}
class B extends Base {}
class C extends Base {}
class D extends Base {}

abstract final class IntContext extends HH\MemoSensitiveImplicitContext {
  const type TData = Base;
  const ctx CRun = [zoned];
  public static async function genStart(this::TData $context, (function (): int) $f)[zoned] {
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: this::TData {
    return parent::get();
  }
}

abstract final class ClassContext extends HH\MemoSensitiveImplicitContext {
  const type TData = Base;
  const ctx CRun = [zoned];
  public static async function genStart(this::TData $context, (function (): int) $f)[zoned] {
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: this::TData {
    return parent::get();
  }
}

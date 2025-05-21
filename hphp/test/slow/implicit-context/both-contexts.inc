<?hh


class Base implements HH\IPureMemoizeParam {
  public int $x;
  public function __construct(int $x) { $this->x = $x; }
  public function getPayload()[zoned]: int { return $this->x; }
  public function getInstanceKey()[]: string {
    return 'KEY' . $this->name();
  }
  public function name()[]: string { return static::class; }
}


abstract final class MemoAgnosticIntCtx extends HH\HHVMTestMemoAgnosticImplicitContext {
  const type TData = int;
  const ctx CRun = [leak_safe];
  public static function start(this::TData $context, (function (): T) $f) [ctx $f, this::CRun] {
    return parent::runWith($context, $f);
  }

  public static function getContext()[leak_safe]: this::TData {
    return parent::exists() ? parent::get() : null;
  }
}

abstract final class MemoSensitiveIntCtx extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = Base;
  const ctx CRun = [zoned];
  public static function start(this::TData $context, (function (): int) $f) {
    return parent::runWith($context, $f);
  }
  public static function getContext(): this::TData {
    return parent::get();
  }
}

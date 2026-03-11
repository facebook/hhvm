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
  public static function start(?this::TData $context, (function (): mixed) $f)[leak_safe, ctx $f]: mixed {
    return parent::runWith($context, $f);
  }
  public static function getContext()[leak_safe]: ?this::TData {
    return parent::get();
  }
  public static function hasContext(): bool {
    return parent::get() is nonnull;
  }
}

abstract final class MemoSensitiveIntCtx extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = Base;
  const ctx CRun = [zoned];
  public static function start(?this::TData $context, (function (): mixed) $f): mixed {
    return parent::runWith($context, $f);
  }
  public static function getContext(): ?this::TData {
    return parent::get();
  }
  public static function hasContext(): bool {
    return parent::get() is nonnull;
  }
}

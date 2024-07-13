<?hh

abstract final class MemoAgnosticIntCtx extends HH\ImplicitContext {
  const type T = int;
  const bool IS_MEMO_SENSITIVE = false;
  const ctx CRun = [defaults];
  public static function runWith(int $context, (function (): T) $f) {
    return parent::runWith($context, $f);
  }

  public static function getContext(): ?int {
    return parent::exists() ? parent::get() : null;
  }
}


abstract final class MemoSensitiveIntCtx extends HH\ImplicitContext {
  const type T = int;
  const bool IS_MEMO_SENSITIVE = true;
  const ctx CRun = [zoned];
  public static function start(int $context, (function (): int) $f) {
    return parent::runWith($context, $f);
  }
  public static function getContext(): ?int {
    return parent::get();
  }
}

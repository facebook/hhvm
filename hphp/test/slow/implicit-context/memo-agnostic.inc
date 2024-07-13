<?hh

abstract final class TestContext extends HH\ImplicitContext {
  const type T = mixed;
  const bool IS_MEMO_SENSITIVE = false;
  const ctx CRun = [defaults];
  public static function runWith(mixed $context, (function (): T) $f) {
    return parent::runWith($context, $f);
  }

  public static function getContext(): ?mixed {
    return parent::exists() ? parent::get() : null;
  }
}

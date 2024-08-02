<?hh

abstract final class TestContext extends HH\ImplicitContext {
  const type T = mixed;
  const bool IS_MEMO_SENSITIVE = false;
  const ctx CRun = [leak_safe];
  public static function runWith(mixed $context, (function (): T) $f)[ctx $f, this::CRun] {
    return parent::runWith($context, $f);
  }

  public static function getContext() [this::CRun]: ?mixed {
    return parent::exists() ? parent::get() : null;
  }
}

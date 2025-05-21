<?hh

abstract final class TestContext extends HH\MemoAgnosticImplicitContext {
  const type TData = mixed;
  const ctx CRun = [leak_safe];
  public static function start(mixed $context, (function (): T) $f)[ctx $f, this::CRun] {
    return parent::runWith($context, $f);
  }

  public static function getContext() [this::CRun]: ?mixed {
    return parent::exists() ? parent::get() : null;
  }
}

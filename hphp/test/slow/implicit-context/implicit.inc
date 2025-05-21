<?hh

class Base implements HH\IPureMemoizeParam {
  public int $x;
  public function __construct(int $x)[] { $this->x = $x; }
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

trait ClassContextDefs {
  const type TData = Base;
  const ctx CRun = [zoned];
  public static function start(this::TData $context, (function (): int) $f)[zoned, ctx $f] {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: this::TData {
    return parent::get();
  }
  public static function exists()[zoned]: bool {
    return parent::exists() as bool;
  }
}

abstract final class ClassContext extends HH\HHVMTestMemoSensitiveImplicitContext {
  use ClassContextDefs;
}

abstract final class ClassContext2 extends HH\HHVMTestMemoSensitiveImplicitContext {
  use ClassContextDefs;
}

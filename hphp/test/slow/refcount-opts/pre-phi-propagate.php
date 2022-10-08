<?hh

interface IFace {
  public function foo(): bool;
  public function bar(): bool;
  public function baz(): bool;
}

abstract class Base {
  abstract public function foo(): bool;
  abstract public function bar(): bool;

  <<__NEVER_INLINE>>
  public function moreCtxRefs(bool $val): bool {
    echo "side effect\n";
    return __hhvm_intrinsics\launder_value($val);
  }

  <<__ALWAYS_INLINE>>
  public function evenMoreCtxRefs(bool $val): bool {
    return $this->moreCtxRefs($val);
  }

  <<__ALWAYS_INLINE>>
  public function baz(): bool {
    return $this->evenMoreCtxRefs(true);
  }
}

class Sub1 extends Base implements IFace {
  <<__ALWAYS_INLINE>>
  public function foo(): bool {
    return $this->evenMoreCtxRefs(true);
  }

  <<__NEVER_INLINE>>
  public function bar(): bool {
    throw new Exception('unreachable');
  }
}

class Sub2 extends Base implements IFace {
  public function __construct(private Base $val) {}

  <<__ALWAYS_INLINE>>
  public function foo(): bool {
    return $this->evenMoreCtxRefs(false);
  }

  <<__NEVER_INLINE>>
  public function bar(): bool {
    echo "side effect\n";
    return $this->val is Sub2;
  }
}

<<__NEVER_INLINE>>
function boom(IFace $b): bool {
  if (!($b->foo() || $b->bar())) return false;
  return $b->baz();
}

<<__EntryPoint>>
function main(): void {
  $sub1 = new Sub1();
  $sub2 = new Sub2($sub1);
  boom($sub1);
  boom($sub2);
  boom($sub2);
}

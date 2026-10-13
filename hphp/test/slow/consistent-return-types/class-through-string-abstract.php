<?hh

class C {}

abstract class Base {
  abstract public function getTypeAbs(): string;
  public function getType(): string { return ''; }
}

final class Derived extends Base {
  public function __construct(private class<mixed> $classType) {}

  public function getTypeAbs(): class<mixed> { return $this->classType; }
  public function getType(): class<mixed> { return $this->classType; }
}

<<__EntryPoint>>
function main(): void {
  foreach (vec[HH\classname_to_class(C::class), C::class] as $cls) {
    $d = new Derived($cls);
    __hhvm_intrinsics\debug_var_dump_lazy_class($d->getTypeAbs());
    __hhvm_intrinsics\debug_var_dump_lazy_class($d->getType());
  }
}

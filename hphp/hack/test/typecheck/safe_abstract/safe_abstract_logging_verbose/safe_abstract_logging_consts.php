<?hh

abstract class C1 {
  abstract const int abs;
  const int concr = 1;
  public function meth(): void {
    static::abs; // use_kind: const_access, is_safe: true
  }
}

final abstract class C2 extends C1 {
  const int abs = 1;
}

function foo(classname<C1> $class): void {
  $class::abs; // use_kind: const_access, is_safe: false
  $class::concr; // use_kind: const_access, is_safe: false
}

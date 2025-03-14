<?hh

// safe_const_accesses":1,"unsafe_const_accesses":0
abstract class C1 {
  abstract const int abs;
  const int concr = 1;
  public function meth(): void {
    static::abs;
  }
}

final abstract class C2 extends C1 {
  const int abs = 1;
}

// safe_const_accesses":0,"unsafe_const_accesses":1
function unsafe(classname<C1> $class): void {
  $class::abs; // use_kind: const_access, is_safe: false
}

// safe_const_accesses":1,"unsafe_const_accesses":0
function safe(classname<C1> $class): void {
  $class::concr;
}

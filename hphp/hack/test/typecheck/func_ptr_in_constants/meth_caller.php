<?hh

function foo(): int { return 42; }

class Cls {
  public function meth(): int { return 42; }
}

class A {
  public static (function(Cls): int) $arr = meth_caller(Cls::class, 'meth');
}

abstract class B {
  const (function(Cls): int) const_arr = meth_caller(Cls::class, 'meth');
}

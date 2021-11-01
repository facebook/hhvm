<?hh

function foo(): int { return 42; }

class Cls {
  public static function meth(): int { return 42; }
}

class A {
  public static dict<string, mixed> $arr = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

abstract class B {
  const dict<string, mixed> const_arr = dict[
    'foo' => fun('foo'),
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

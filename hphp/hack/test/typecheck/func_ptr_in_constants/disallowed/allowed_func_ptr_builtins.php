<?hh

function foo(): int { return 42; }

class Cls {
  public static function meth(): int { return 42; }
}

class A {
  public static dict<string,mixed> $arr = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}

abstract class B {
  const dict<string,mixed> const_arr = dict[
    'foo' => foo<>,
    'meth' => Cls::meth<>,
  ];
}

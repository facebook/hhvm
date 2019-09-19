<?hh

class Cls {
  public static function meth() {}
}

class A {
  const f = dict[
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

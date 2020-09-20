<?hh

class Cls {
  public static function meth() {}
}

class A {
  public static $arr = dict[
    'meth' => class_meth(Cls::class, 'meth'),
  ];
}

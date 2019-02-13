<?hh

class A { function f($x) { return $x; } }
class B extends A {
  function f($x) { return $x * 2; }

  static function specail_classes() {
    var_dump(HH\meth_caller(parent::class, "f")(new A(), 1));
    var_dump(HH\meth_caller(parent::class, "f")(new B(), 1));
    var_dump(HH\meth_caller(static::class, "f")(new B(), 2));
    var_dump(HH\meth_caller(self::class, "f")(new B(), 3));
  }
}

new B()->specail_classes();

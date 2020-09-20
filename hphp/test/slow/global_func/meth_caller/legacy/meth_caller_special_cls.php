<?hh

class A { function f($x) { return $x; } }
class B extends A {
  function f($x) { return $x * 2; }

  static function specail_classes() {
    $x = HH\meth_caller(parent::class, "f");
    var_dump(
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
    var_dump($x(new A(), 1));
    var_dump($x(new B(), 1));

    $x = HH\meth_caller(static::class, "f");
    var_dump(
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
    var_dump($x(new B(), 2));

    $x = HH\meth_caller(self::class, "f");
    var_dump(
      \HH\meth_caller_get_class($x), \HH\meth_caller_get_method($x));
    var_dump($x(new B(), 3));
  }
}
<<__EntryPoint>> function main(): void {
B::specail_classes();
}

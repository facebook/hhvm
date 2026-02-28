<?hh

class A { function f($x) :mixed{ return $x; } }
class B extends A {
  function f($x) :mixed{ return $x * 2; }

  static function specail_classes() :mixed{
    // These classes will not be supported
    var_dump(HH\meth_caller(parent::class, "f")(new B(), 2));
    var_dump(HH\meth_caller(static::class, "f")(new B(), 2));
    var_dump(HH\meth_caller(self::class, "f")(new B(), 3));
  }
}
<<__EntryPoint>> function main(): void {
new B()->specail_classes();
}

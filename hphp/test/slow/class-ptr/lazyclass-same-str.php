<?hh

abstract class Base {}

class C1 extends Base {}
class C2 extends Base {}

class Foo {
  public static darray<int, classname<Base>> $p = dict[
    1 => C1::class,
    3 => C2::class,
  ];

  public static function baz(classname<Base> $name) : int {
    foreach (Foo::$p as $ii => $cur) {
      if ($cur === $name) {
        return $ii;
      }
    }
    return 0;
  }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(__hhvm_intrinsics\launder_value(Foo::baz(C2::class)));
  var_dump(Foo::baz(C2::class));
}

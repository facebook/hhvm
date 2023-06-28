<?hh

abstract final class Foo {
  public static function get() : dict<classname, classname> {
    return dict[
      A::class => B::class,
      C::class => D::class,
    ];
  }
}

abstract class Bar {
  final public static function getClass() : classname {
    return idx(Foo::get(), static::class);
  }
}

final class C extends Bar {
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::getClass());
}

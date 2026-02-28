<?hh

class Base {
  public static vec<string> $side_effects = vec[];

  <<__MemoizeLSB>>
  public static function name(): string {
    $ret = static::class;
    self::$side_effects[] = $ret;
    return $ret;
  }
}

class A extends Base { }
class B extends Base { }
class C extends Base { }

class A1 extends A { }
class A2 extends A { }

function main() :mixed{
  Base::$side_effects = vec[];
  var_dump(vec[
    A1::name(),
    B::name(),
    C::name(),
    A::name(),
    A2::name(),
    Base::name(),
  ]);
  var_dump(Base::$side_effects);
}
<<__EntryPoint>> function main_entry(): void {
main();
main();
}

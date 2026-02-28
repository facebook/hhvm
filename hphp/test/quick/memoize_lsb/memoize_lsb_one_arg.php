<?hh

class Base {
  public static vec<string> $side_effects = vec[];

  <<__MemoizeLSB>>
  public static function name(string $suffix): string {
    $ret = static::class . $suffix;
    self::$side_effects[] = $ret;
    return $ret;
  }
}

class A extends Base { }
class B extends Base { }
class C extends Base { }

class D extends A { }
class E extends A { }

function main($suffix) :mixed{
  Base::$side_effects = vec[];
  var_dump(vec[
    B::name($suffix),
    D::name($suffix),
    C::name($suffix),
    A::name($suffix),
    E::name($suffix),
    Base::name($suffix),
  ]);
  var_dump(Base::$side_effects);
}
<<__EntryPoint>> function main_entry(): void {
main("X");
main("Y");

main("X");
main("Y");
}

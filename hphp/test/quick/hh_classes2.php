<?hh

class :x:frag {}

class Foo {
  private :x:frag $c;
  protected :x:frag $b;
  public :x:frag $a;

  public static function bar(:x:frag $x): :x:frag {
    return $x;
  }
}
print_r(Foo::bar(<x:frag/>));

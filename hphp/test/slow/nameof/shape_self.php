<?hh

namespace N {
  class C {
    public static function f(): void {
      $s = shape('N\C' => "ns_str");
      $l = shape(C::class => "ns_lazy");
      $n = shape(nameof C => "ns_nameof");

      \var_dump($s[self::class]);
      \var_dump($s[nameof self]);
      \var_dump(Shapes::idx($s, self::class));
      \var_dump(Shapes::idx($s, nameof self));

      \var_dump($l[self::class]);
      \var_dump($l[nameof self]);
      \var_dump(Shapes::idx($l, self::class));
      \var_dump(Shapes::idx($l, nameof self));

      \var_dump($n[self::class]);
      \var_dump($n[nameof self]);
      \var_dump(Shapes::idx($n, self::class));
      \var_dump(Shapes::idx($n, nameof self));
    }
  }
}

namespace {
  class C {
    public static function f(): void {
      $s = shape('C' => "ns_str");
      $l = shape(C::class => "ns_lazy");
      $n = shape(nameof C => "ns_nameof");

      \var_dump($s[self::class]);
      \var_dump($s[nameof self]);
      \var_dump(Shapes::idx($s, self::class));
      \var_dump(Shapes::idx($s, nameof self));

      \var_dump($l[self::class]);
      \var_dump($l[nameof self]);
      \var_dump(Shapes::idx($l, self::class));
      \var_dump(Shapes::idx($l, nameof self));

      \var_dump($n[self::class]);
      \var_dump($n[nameof self]);
      \var_dump(Shapes::idx($n, self::class));
      \var_dump(Shapes::idx($n, nameof self));
    }
  }

  <<__EntryPoint>>
  function main(): void {
    C::f();
    N\C::f();
  }
}

<?hh

enum FooEnum: int {
  A = 1;
  B = 2;
  C = 3;
  D = 4;
  E = 5;
  F = 6;
}

final class Foo {
  const dict<FooEnum, dict<int, keyset<int>>> DICT = dict[
    FooEnum::A => dict[
      0 => keyset[1],
    ],
    FooEnum::B => dict[
      0 => keyset[2],
    ],
    FooEnum::C => dict[
      0 => keyset[3],
    ],
    FooEnum::D => dict[
      0 => keyset[4],
    ],
    FooEnum::E => dict[],
    FooEnum::F => dict[
      0 => keyset[5],
    ],
  ];

  <<__NEVER_INLINE>>
  public static function foo(int $x): keyset<int> {
    $var = null;
    return self::DICT[$x] |> $$[$var ?? 0] ?? keyset[];
  }
}

<<__EntryPoint>>
function main(): void {
  for ($i = 0; $i < 100; ++$i) {
    Foo::foo(__hhvm_intrinsics\launder_value(FooEnum::A));
    Foo::foo(__hhvm_intrinsics\launder_value(FooEnum::B));
    Foo::foo(__hhvm_intrinsics\launder_value(FooEnum::C));
    Foo::foo(__hhvm_intrinsics\launder_value(FooEnum::D));
    Foo::foo(__hhvm_intrinsics\launder_value(FooEnum::F));
  }
  var_dump(Foo::foo(__hhvm_intrinsics\launder_value(FooEnum::E)));
}

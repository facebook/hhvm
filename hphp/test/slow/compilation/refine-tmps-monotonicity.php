<?hh

class Foo {
  public static ?string $val = null;

  public static function one(): ?string {
    $val = HH\Readonly\as_mut(readonly self::$val);
    return (bool)$val ? $val : null;
  }

  private static function two(): ?string {
    $val = HH\Readonly\as_mut(readonly self::$val);
    return $val === null || $val === '' ? null : $val;
  }

  public static function test(): ?string {
    $one = self::one();
    $two = self::two();

    if ($one !== $two) {
      var_dump('fail');
    }

    return $one;
  }
}

<<__EntryPoint>>
function main() {
  HH\Readonly\as_mut(__hhvm_intrinsics\launder_value(null));
  Foo::$val = 'hello world';
  var_dump(Foo::test());
}

<?hh

class A {
  const string X = 'a';
}

class C extends A {
  public function staticConst(string $s = static::X): void {}
  public function staticClass(string $s = static::class): void {}
  public function nameofStatic(string $s = nameof static): void {}
}

trait T {
  require extends A;

  const string Y = 't';

  public function selfConst(string $s = self::Y): void {}
  public function selfClass(string $s = self::class): void {}
  public function parentConst(string $s = parent::X): void {}

  attribute string static_const = static::Y;
}

class :x extends A {
  attribute string static_const = static::X, string static_class = static::class;
}

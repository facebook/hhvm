<?hh

class A {
  const string X = 'a';
}

class C extends A {
  public function staticConst(string $s = static::X): void {}
  public function staticClass(string $s = static::class): void {}
  public function nameofStatic(string $s = nameof static): void {}
  public function parentClass(string $s = parent::class): void {}
  public function nameofParent(string $s = nameof parent): void {}
  public function nested(
    vec<string> $v = vec[static::X],
    shape('a' => string) $s = shape('a' => nameof static),
  ): void {}
  public function lambda(): void {
    $f = (string $s = static::X) ==> $s;
    $g = (string $s = nameof static) ==> $s;
  }
}

interface I {
  const string X = 'i';

  public function staticConst(string $s = static::X): void;
}

class :x extends A {
  attribute
    string static_const = static::X,
    string static_class = static::class,
    string nameof_static = nameof static,
    vec<string> nested = vec[static::X];
}

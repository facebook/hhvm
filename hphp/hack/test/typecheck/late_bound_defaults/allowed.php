<?hh

class A {
  const string X = 'a';
}

class C extends A {
  public function named(string $s = A::class, string $t = nameof A): void {}
  public function selfForms(
    string $s = self::class,
    string $t = nameof self,
    string $u = self::X,
  ): void {}
  public function parentConst(string $s = parent::X): void {}
  public function body(): vec<string> {
    return vec[
      static::X,
      static::class,
      nameof static,
      self::class,
      nameof self,
      parent::class,
      nameof parent,
    ];
  }
}

interface I {
  const string X = 'i';

  public function selfForms(
    string $s = self::class,
    string $t = nameof self,
    string $u = self::X,
  ): void;
}

trait T {
  const string X = 't';

  public string $selfClassProp = self::class;
  public string $selfConstProp = self::X;

  public function named(string $s = A::class, string $t = nameof A): void {}
  public function body(): vec<string> {
    return vec[static::X, static::class, nameof static, self::class, self::X];
  }

  attribute string self_class = self::class, string self_const = self::X;
}

class :x extends A {
  attribute
    string a_class = A::class,
    string nameof_a = nameof A,
    string self_const = self::X;
}

function top_level(string $s = A::class, string $t = nameof A): void {}

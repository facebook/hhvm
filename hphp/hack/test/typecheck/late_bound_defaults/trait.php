<?hh

class P {
  const string X = 'p';
}

trait T {
  require extends P;

  const string Y = 't';

  public function selfConst(string $s = self::Y): void {}
  public function selfClass(string $s = self::class): void {}
  public function nameofSelf(string $s = nameof self): void {}
  public function nameofSelfUpper(string $s = nameof Self): void {}
  public function parentConst(string $s = parent::X): void {}
  public function nameofParentUpper(string $s = nameof Parent): void {}
  public function staticConst(string $s = static::Y): void {}
  public function lambda(): void {
    $f = (string $s = self::Y) ==> $s;
  }

  attribute string static_const = static::Y;
}

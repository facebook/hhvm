<?hh // partial

class Example {
  public function __callStatic(string $name, string $args): string {
    return 'parent test';
  }

  public function __call(string $name, string $args): string {
    return 'invoke parent';
  }
}

class SecondExample extends Example {
  public function __call(string $name, string $args): string {
    parent::__callStatic('one', 'two');
    return 'test';
  }
}

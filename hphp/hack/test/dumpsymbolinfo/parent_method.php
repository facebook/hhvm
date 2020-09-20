<?hh

class C1 {
  public static function foo(): string {
    return 'foo';
  }
}

class C2 extends C1 {
  public static function test(): string {
    return parent::foo();
  }
}

class C3 extends C2 {
  public static function test2(): string {
    return parent::foo();
  }
}

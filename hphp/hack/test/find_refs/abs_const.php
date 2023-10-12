<?hh // strict
abstract class C {
  abstract const string FOO;

  public function test(): void {
    static::FOO;
  }
}

class D extends C {
  const FOO = "aa";
}

function test(): void {
  D::FOO;
}

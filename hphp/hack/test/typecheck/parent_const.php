<?hh

class C1 {
  const int foo = 10;

  final public function foo(): string {
    return 'foo';
  }
}

class C2 extends C1 {
  public function test(): string {
    return parent::foo;
  }
}

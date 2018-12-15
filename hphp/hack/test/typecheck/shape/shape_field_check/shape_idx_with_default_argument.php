<?hh // strict

class TestClass {
  const string ID = 'id';
  const string X = 'x';

  const type TClassType = shape(
    self::ID => int,
    'x' => int,
  );

  public function test(self::TClassType $s): void {
    Shapes::idx($s, 'xy', 0);
  }
}

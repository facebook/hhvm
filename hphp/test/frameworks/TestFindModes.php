<?hh
type TestFindMode = string;
class TestFindModes {
  const TestFindMode REFLECTION = "reflection";
  const TestFindMode TOKEN = "token";
  const TestFindMode PHPT = "phpt";

  public static function assertIsValid(string $token) {
    assert(
      $token === self::REFLECTION
      || $token === self::TOKEN
      || $token == self::PHPT
    );
  }
}

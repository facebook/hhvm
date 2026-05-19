<?hh

function takes_string(string $_): void {}

class MyClass {
  public function fromDynamic(dynamic $d): void {
    takes_string($d);
  }

  public static function staticFromDynamic(dynamic $d): int {
    return $d;
  }
}

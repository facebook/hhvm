<?hh

function expect_int(int $_)[]: void {}

final class Subject {
  public ?int $value = 1;

  public function pure((function()[]: void) $f): void {
    if ($this->value !== null) {
      $f();
      expect_int($this->value);
    }
  }

  public function globals((function()[globals]: void) $f): void {
    if ($this->value !== null) {
      $f();
      expect_int($this->value);
    }
  }

  public function writing((function()[write_props]: void) $f): void {
    if ($this->value !== null) {
      $f();
      expect_int($this->value); // error: $f may write properties
    }
  }
}

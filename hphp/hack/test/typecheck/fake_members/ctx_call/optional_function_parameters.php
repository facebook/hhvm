<?hh

function expect_int(int $_)[]: void {}

function call_optional(?(function()[_]: void) $f)[ctx $f]: void {
  if ($f !== null) {
    $f();
  }
}

final class Subject {
  public ?int $value = 1;

  public function pureFunctionParameter(?(function()[]: void) $f): void {
    if ($this->value !== null) {
      call_optional($f);
      expect_int($this->value);
    }
  }

  public function writingFunctionParameter(
    ?(function()[write_props]: void) $f,
  ): void {
    if ($this->value !== null) {
      call_optional($f);
      // Currently accepted despite the function's declared capability.
      expect_int($this->value);
    }
  }
}

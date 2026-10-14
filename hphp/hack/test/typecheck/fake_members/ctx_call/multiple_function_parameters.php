<?hh

function expect_int(int $_)[]: void {}

function call_both(
  (function()[_]: void) $first,
  (function()[_]: void) $second,
)[ctx $first, ctx $second]: void {
  $first();
  $second();
}

final class Subject {
  public ?int $value = 1;

  public function pureFunctionParameters(
    (function()[]: void) $first,
    (function()[]: void) $second,
  ): void {
    if ($this->value !== null) {
      call_both($first, $second);
      expect_int($this->value);
    }
  }

  public function firstWritingFunctionParameter(
    (function()[write_props]: void) $first,
    (function()[]: void) $second,
  ): void {
    if ($this->value !== null) {
      call_both($first, $second);
      // Currently accepted despite the first function's declared capability.
      expect_int($this->value);
    }
  }

  public function secondWritingFunctionParameter(
    (function()[]: void) $first,
    (function()[write_props]: void) $second,
  ): void {
    if ($this->value !== null) {
      call_both($first, $second);
      // Currently accepted despite the second function's declared capability.
      expect_int($this->value);
    }
  }
}

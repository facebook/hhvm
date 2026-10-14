<?hh

function expect_int(int $_)[]: void {}

function call_polymorphic((function()[_]: void) $f)[ctx $f]: void {
  $f();
}

function accept_writing((function()[write_props]: void) $_): void {}

function pure_function()[]: void {}

function writing_function()[write_props]: void {}

function pure_is_a_sub_effect(): void {
  accept_writing(()[] ==> {});
}

final class Subject {
  public ?int $value = 1;

  public function pureFunctionParameter((function()[]: void) $f): void {
    if ($this->value !== null) {
      call_polymorphic($f);
      expect_int($this->value);
    }
  }

  public function globalsFunctionParameter((function()[globals]: void) $f): void {
    if ($this->value !== null) {
      call_polymorphic($f);
      expect_int($this->value);
    }
  }

  public function writingFunctionParameter(
    (function()[write_props]: void) $f,
  ): void {
    if ($this->value !== null) {
      call_polymorphic($f);
      // Currently accepted despite the function's declared capability.
      expect_int($this->value);
    }
  }

  public function defaultsFunctionParameter((function(): void) $f): void {
    if ($this->value !== null) {
      call_polymorphic($f);
      // Currently accepted despite the function's declared capability.
      expect_int($this->value);
    }
  }

  public function effectPolymorphicFunctionParameter(
    (function()[_]: void) $f,
  )[ctx $f]: void {
    if ($this->value !== null) {
      call_polymorphic($f);
      // Currently accepted although the caller may supply a writing function.
      expect_int($this->value);
    }
  }

  public function pureFunctionLiteral(): void {
    if ($this->value !== null) {
      call_polymorphic(()[] ==> {});
      expect_int($this->value);
    }
  }

  public function writingFunctionLiteral(): void {
    if ($this->value !== null) {
      call_polymorphic(() ==> { $this->value = null; });
      // Currently accepted even though the function writes this property.
      expect_int($this->value);
    }
  }

  public function inheritedContextFunctionLiteral(): void {
    if ($this->value !== null) {
      call_polymorphic(() ==> {});
      // The function literal inherits [defaults], despite having a pure body.
      expect_int($this->value);
    }
  }

  public function functionPointers(): void {
    if ($this->value !== null) {
      call_polymorphic(pure_function<>);
      call_polymorphic(writing_function<>);
      // Currently accepted despite the second function's context.
      expect_int($this->value);
    }
  }
}

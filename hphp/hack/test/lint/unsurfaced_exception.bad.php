<?hh

class UnsurfacedExceptionLintRuleTestException extends Exception {}

class C {
  function bad(): void {
    try {
      $this->stuff(null);
    } catch (ViolationException $_) {
      $this->stuff(null);
    }
  }

  public function stuff(): void {}
}

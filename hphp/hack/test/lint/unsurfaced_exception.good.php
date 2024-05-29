<?hh

class UnsurfacedExceptionLintRuleTestException extends Exception {}

class C {
  function good(): void {
    try { // OK b/c specific exception
      $this->stuff(null);
    } catch (UnsurfacedExceptionLintRuleTestException $_) {
      $this->stuff(null);
    }

    try { // OK b/c rethrow
      $this->stuff(null);
    } catch (ViolationException $e) {
      throw $e;
    }

    try { // OK b/c used
      $this->stuff(null);
    } catch (Exception $e) {
      $this->stuff($e);
    }
  }

  public function stuff(): void {}
}

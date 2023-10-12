<?hh

interface I1 {
  function i1(): void; // legal
  public function i2(): void; // legal
  private function i3(): void; // error2047
  protected function i4(): void; // error2047
  static final function i1(): void; // legal
  static final public function i2(): void; // legal
  static final private function i3(): void; // error2047
  static final protected function i4(): void; // error2047
}

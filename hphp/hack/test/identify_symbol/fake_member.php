<?hh

class C {
  private ?int $foo = null;

  public function get(): void {
    if ($this->foo) {
      // TODO(t11082787): this property is not correctly identified, probably some bug
      // related to FakeMembers
      $this->foo;
    }
  }
}

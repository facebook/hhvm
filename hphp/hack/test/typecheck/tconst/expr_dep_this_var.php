<?hh

abstract class C {
  abstract const type T;

  abstract public function get(): this::T;

  public function test(): void {
    if (true) {
      // Force integration of local variables
    }
    hh_show($this);
    hh_show($this->get());
  }
}

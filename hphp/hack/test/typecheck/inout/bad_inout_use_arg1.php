<?hh

function f(inout int $i): void {}

class C {
  private int $x = 42;

  public function test(): void {
    f(inout $this->x);
  }
}

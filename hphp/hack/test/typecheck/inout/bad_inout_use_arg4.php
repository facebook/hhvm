<?hh

function f(inout int $i): void {}

class C {
  private vec<int> $x = vec[3, 42];

  public function test(): void {
    f(inout $this->x[1]);
  }
}

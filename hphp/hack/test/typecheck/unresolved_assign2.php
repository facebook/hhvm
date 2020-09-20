<?hh // strict

function expects_int(int $_): void {}

class C {

  public function __construct(private vec<mixed> $x) {}

  public function test(): void {
    hh_show($this->x);
    foreach (vec[] as $_) {
    }
    hh_show($this->x);
    $this->x = vec[1, 2, 3];
    foreach ($this->x as $v) {
      expects_int($v);
    }
  }
}

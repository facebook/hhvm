<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {
  public function f()@{ int }: void {
    $this->f(); // ok
    $this->g(); // ok
  }

  public function g()@{ arraykey }: void {
    $this->f(); // error, arraykey </: int
    $this->g();
  }

  public function g_unsafe()@{ arraykey + int }: void {
    $this->f(); // unsafely ok, (arraykey&int) <: int
    $this->g();
  }
}

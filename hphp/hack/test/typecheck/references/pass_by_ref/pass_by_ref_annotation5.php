<?hh // partial

function f(&$x) {}

class C {
  public function test() {
    f(&$this);
    10 |> f(&$$);
  }
}

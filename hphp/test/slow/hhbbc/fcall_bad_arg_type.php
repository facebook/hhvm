<?hh

class C {
  public vec<int> $xs;

  function foo(int $x): this {
    $this->xs[] = $x;
    return $this;
  }
}

<<__EntryPoint>>
function main() {
  $c = new C();
  var_dump($c->foo(null));
}

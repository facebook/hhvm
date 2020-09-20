<?hh

class Y {
  const A = 'a';
}

class A {
  public ImmMap $pub;
  protected ImmMap $prot;
}

class X extends A {
  public ImmMap $pub = ImmMap {
    Y::A => 42,
  };
  protected ImmMap $prot = ImmMap {
    Y::A => 24,
  };

  function getPub() { return $this->pub; }
  function getProt() { return $this->prot; }
}

function test() {
  return new X;
}

<<__EntryPoint>> function main(): void {
  for ($i = 1; $i < 100; $i++) {
    test();
  }
  $x = test();
  var_dump($x->getPub(), $x->getProt());
}

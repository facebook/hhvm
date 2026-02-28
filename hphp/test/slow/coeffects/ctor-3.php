<?hh

class C {
  public function __construct(private int $x)[] {}
  public function set(int $x)[write_this_props] :mixed{ $this->x = $x; }
}

// should not be allowed to modify $c
function pure(C $c)[] :mixed{
  $c->__construct(42); // ban
  $c->set(43); // ban
  $c = new C(44); // allow
}

<<__EntryPoint>>
function main() :mixed{
  pure(new C(1));
}

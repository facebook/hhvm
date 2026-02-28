<?hh

abstract class one {
  abstract protected function foo():mixed;
}

class a extends one {
  protected function foo() :mixed{ echo "a\n"; }
}

class b extends one {
  protected function foo() :mixed{ echo "b\n"; }
}

class c extends one {
  protected function foo() :mixed{ echo "c\n"; }

  public function go($x) :mixed{
    $x->foo();
  }
}

<<__EntryPoint>> function main(): void {
  $a = new a;
  $b = new b;
  $c = new c;
  $c->go($a);  // fill
  $c->go($a);  // hit
  $c->go($b);  // would call with not AttrPublic
  $c->go($c);  // again
  $c->go($c);  // hit
  $c->go($a);  // would call, not attr public
}

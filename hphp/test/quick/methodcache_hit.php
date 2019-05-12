<?hh

class one {
  public function foo() { echo "foo\n"; }
}
class doer {
  public function junk($x) {
    $x->foo();
  }
}

<<__EntryPoint>> function main(): void {
  $x = new doer;
  $o = new one;
  $x->junk($o);
  $x->junk($o);
  $x->junk($o);
}

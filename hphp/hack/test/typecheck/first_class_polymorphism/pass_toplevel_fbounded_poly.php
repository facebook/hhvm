<?hh

interface I<T> {
  public function do_it(): void;
}

function fbounded<T as I<T>>(T $x): void {
  $x->do_it();
}

class C implements I<C> {
  public function do_it(): void {}
}

function rcvr((function(C): void) $_): void {}

function pass_generic(): void {
  $f = fbounded<>;
  rcvr($f);
}

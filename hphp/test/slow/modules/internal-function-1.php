<?hh

module B;

class C {
  public function getFoo(): void {
    foo();
  }
}

<<__EntryPoint>>
function bar(): void {
  include 'internal-function-1.inc0';
  include 'internal-function-1.inc1';
  include 'internal-function-1.inc2';

  (new C())->getFoo();
}

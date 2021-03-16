<?hh

function pure()[] { echo "in pure\n"; }
function rx()[rx] { echo "in rx\n"; }
function defaults() { echo "in defaults\n"; }

abstract class A {
  abstract const ctx C;
  public function f()[this::C] {
    pure();
    rx();
    defaults();
  }
}

class B1 extends A { const ctx C = []; }
class B2 extends A { const ctx C = [rx]; }
class B3 extends A { const ctx C = [defaults]; }

<<__EntryPoint>>
function main() {
  (new B1)->f();
  (new B2)->f();
  (new B3)->f();
}

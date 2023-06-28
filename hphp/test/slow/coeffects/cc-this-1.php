<?hh

abstract class A {
  abstract const ctx C;
  public function f()[this::C] :mixed{
    echo "in f\n";
  }
}

class B1 extends A { const ctx C = []; }
class B2 extends A { const ctx C = [rx]; }
class B3 extends A { const ctx C = [defaults]; }

function pure($c)[]   :mixed{ $c->f(); }
function rx($c)[rx]   :mixed{ $c->f(); }
function defaults($c) :mixed{ $c->f(); }


<<__EntryPoint>>
function main() :mixed{
  $classes = vec[new B1, new B2, new B3];
  $callers = vec['pure', 'rx', 'defaults'];
  foreach ($callers as $caller) {
    foreach ($classes as $cls) {
      $caller($cls);
    }
  }
}

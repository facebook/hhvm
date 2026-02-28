<?hh

class C {}

class D {
  private $c;

  public function __construct(?C $c) {
    $this->c = $c ?? shape();
  }
}

<<__EntryPoint>>
function main() :mixed{
  new D(new C());
  echo "OK\n";
}

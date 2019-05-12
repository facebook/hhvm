<?hh

trait t {
  public static function f($o) {
    $o->blah();
  }
}

class c {
  use t;
  private function blah() {
    echo "private function\n";
  }
}

<<__EntryPoint>> function main(): void {
  $o = new c;
  c::f($o);
  t::f($o);
}

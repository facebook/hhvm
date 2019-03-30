<?hh

if (getenv("NOFOO")) {
  include 'non-unique-base-1.inc';
} else {
  include 'non-unique-base-2.inc';
}

trait T {
  abstract protected function foo();
  public function main() { $this->foo(); }
}

class C extends B {
  use T;
}

$obj = new C;
$obj->main();

<?hh
trait T {
  protected function f() {
    return 1;
  }
  private function p() {
    return 2;
  }
  function g() {
    return $this->f();
  }
  function h() {
    return $this->p();
  }
}
class C {
  use T;
}


<<__EntryPoint>>
function main_2058() {
error_reporting(E_ALL | E_STRICT);
$c = new C;
echo $c->g();
echo $c->h();
}

<?hh
trait T {
  protected function f() :mixed{
    return 1;
  }
  private function p() :mixed{
    return 2;
  }
  function g() :mixed{
    return $this->f();
  }
  function h() :mixed{
    return $this->p();
  }
}
class C {
  use T;
}


<<__EntryPoint>>
function main_2058() :mixed{
error_reporting(E_ALL | E_STRICT);
$c = new C;
echo $c->g();
echo $c->h();
}

<?hh
class C {
  public $a;
  public $b;
  public function f() {
    $this->a = new stdClass();
    $this->b = 1;
    $this->a = 2;
    $this->b = new stdClass();
    $this->a = null;
    $this->b = null;
  }
}
<<__EntryPoint>> function main(): void {
$obj = new C();
$obj->f();
}

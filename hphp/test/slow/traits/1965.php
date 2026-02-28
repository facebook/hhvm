<?hh

trait T1 {
  public $y = 3;
}
trait T2 {
  public $x = 4;
}
class C {
  use T1, T2;
  public function printProps() :mixed{
    var_dump($this->y);
    var_dump($this->x);
  }
}
<<__EntryPoint>> function main(): void {
$o = new C;
$o->printProps();
}

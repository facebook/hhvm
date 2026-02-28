<?hh

trait T1 {
  public $x = 3;
}
trait T2 {
  use T1;
}
trait T3 {
  use T1;
}
class C {
  use T2, T3;
  public function printProps() :mixed{
    var_dump($this->x);
  }
}
<<__EntryPoint>> function main(): void {
$o = new C;
$o->printProps();
}

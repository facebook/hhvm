<?hh

trait T1 {
  public $x = 1977;
}
trait T2 {
  public $x = 1977;
}
class MY_CLASS {
  use T1, T2;
  public $abc = 1;
  public function printProps() :mixed{
    var_dump($this->x);
  }
}
<<__EntryPoint>> function main(): void {
  $o = new MY_CLASS;
  $o->printProps();
}

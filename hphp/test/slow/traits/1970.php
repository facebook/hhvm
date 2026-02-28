<?hh

class MY_BASE {
  public $x = 3;
}
trait MY_TRAIT {
  public $x = 4;
}
class MY_CLASS extends MY_BASE {
  use MY_TRAIT;
  public $x = 4;
  public function printX() :mixed{
    var_dump($this->x);
  }
}
<<__EntryPoint>> function main(): void {
$o = new MY_CLASS;
$o->printX();
}

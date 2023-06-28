<?hh

trait MY_TRAIT {
  public $x;
}
class MY_CLASS{
  use MY_TRAIT;
  public $x;
  public function printX() :mixed{
    var_dump($this->x);
    $this->x = 10;
    var_dump($this->x);
  }
}
<<__EntryPoint>> function main(): void {
$o = new MY_CLASS;
$o->printX();
}

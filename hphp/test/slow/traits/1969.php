<?hh

trait MY_TRAIT {
  static public $x = 3;
}
class MY_CLASS{
  use MY_TRAIT;
  public function printX() :mixed{
    var_dump(self::$x);
  }
}
<<__EntryPoint>> function main(): void {
$o = new MY_CLASS;
$o->printX();
}

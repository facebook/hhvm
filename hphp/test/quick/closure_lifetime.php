<?hh

class blah {
  private string $t = "";
  public function __construct() { $this->t = "hi\n"; }
  public function foo() :mixed{ return function() { return $this->t; }; }
}

<<__EntryPoint>> function main(): void {
  $k = (new blah)->foo(); // only reference to obj is in the closure
  echo $k();
  unset($k);
  echo "done\n";
}

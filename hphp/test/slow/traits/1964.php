<?hh

trait T {
  public $y = 3;
}
class C {
  public $x = 10;
  use T;
  public function printY() :mixed{
    echo "x = " . $this->x . "\n";
    echo "y = " . $this->y . "\n";
  }
}
class D {
  public $y = 4;
}
<<__EntryPoint>> function main(): void {
$o = new C;
$o->printY();
}

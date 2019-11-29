<?hh
class Base {
  public $x;
  public function __construct() { $this->x = 10; }
}

class Derived extends Base {
}

class C {
  public $x;
  public function __construct() { $this->x = 20; }
}

record A {
  Base f;
}
<<__EntryPoint>> function main(): void {
$a = A['f'=>new Derived()];
\var_dump($a['f']->x);

$b = A['f'=>new C()];
}

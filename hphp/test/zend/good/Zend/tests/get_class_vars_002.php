<?hh

class A {
  public $a = 1;
  private $b = 2;
  private $c = 3;
}

class B extends A {
  static public $aa = 4;
  static private $bb = 5;
  static protected $cc = 6;
}

class C extends B {
  public function __construct() {
    var_dump(get_class_vars('A'));
    var_dump(get_class_vars('B'));
    var_dump($this->a);
    try {
      var_dump($this->b);
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this->c);
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
  }
}
<<__EntryPoint>>
function main(): void {
  new C;
}

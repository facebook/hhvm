<?hh // strict

<<__PPL>>
class MyBase {
  public static function staticMeth(): void {
  }
}

<<__PPL>>
class MyClass extends MyBase {
  public function __construct() {
    parent::__construct();
  }

  public function parentConstructor(): void {
    parent::__construct();
  }
}

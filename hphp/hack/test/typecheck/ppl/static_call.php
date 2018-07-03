<?hh // strict

<<__PPL>>
class MyBase {
  public static function staticMeth(): void {
  }
}

<<__PPL>>
class MyClass extends MyBase {
  public function staticMethTest(): void {
    self::staticMeth();
    static::staticMeth();
    parent::staticMeth();
  }
}

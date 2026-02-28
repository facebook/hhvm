<?hh

class E {
  public private function f () : void {}
  final final function f () : void {}
  public abstract final function f () : void ;
  private abstract function f () : void ;
  public abstract static function f () : void {}
  public abstract static function f () : void ;
  public function f () : void ;
  public static function __construct () {}
  public function __construct (public int $k) {}
  public abstract function __construct () ;
  public abstract function __construct () {}
  public function f (public int $k) : void {}
  public function __construct () : int {}
  public function __construct () : void {}
  public function f (...) : void {} // no errors
  public function f (...,) : void {} // error; trailing comma illegal
  public function f (..., int $x) : void {} // error; ... must be at end
}

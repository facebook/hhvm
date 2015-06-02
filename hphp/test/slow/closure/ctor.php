<?hh

class X extends Closure {
  public function __construct() {}
  public static function __invoke() { return 42; }
}
$x = new X();
echo $x();

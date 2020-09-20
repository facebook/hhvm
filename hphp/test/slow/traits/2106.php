<?hh

trait PrivateState {
  private $foo = 2;
  public function getFoo() {
    return $this->foo;
  }
}
class Base {
  public static function create() {
    return new static();
  }
}
class UsePrivateState extends Base {
  use PrivateState;
}
class DerivedUsePrivateState extends UsePrivateState {
}

<<__EntryPoint>>
function main_2106() {
$state = new DerivedUsePrivateState();
$method = new ReflectionMethod('DerivedUsePrivateState', 'getFoo');
echo $method->invoke($state)."\n";
}

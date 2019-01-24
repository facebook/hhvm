<?hh

class A extends DateTime {
  public function test() {
    $x = 'createFromFormat';
    self::$x('j-M-Y', '05-Nov-2017');
    static::$x('j-M-Y', '05-Nov-2017');
    parent::$x('j-M-Y', '05-Nov-2017');
    $x = 'getTimezone'; $this->$x();
  }
}

function test() {
  $x = 'count'; $x([]);

  $x = 'HH\Vector::fromItems'; $x([]);
  $x = ['HH\Vector', 'fromItems']; $x([]);
  $x = [new Vector, 'fromItems']; $x([]);
  $x = [new Vector, 'toArray']; $x();
  $x = 'HH\Vector'; $x::fromItems([]);
  $x = 'fromItems'; Vector::$x([]);
  $x = 'fromItems'; $obj = new Vector; $obj->$x([]);
  $x = 'toArray'; $obj = new Vector; $obj->$x();
  $x = 'HH\Vector'; new $x();

  $obj = new A; $obj->test();

  $x = 'SplFixedArray::fromArray'; $x([1, 2, 3]);
  $x = ['SplFixedArray', 'fromArray']; $x([1, 2, 3]);
  $x = [new SplFixedArray, 'fromArray']; $x([1, 2, 3]);
  $x = [new SplFixedArray, 'toArray']; $x();
  $x = 'SplFixedArray'; $x::fromArray([1, 2, 3]);
  $x = 'fromArray'; SplFixedArray::$x([1, 2, 3]);
  $x = 'fromArray'; $obj = new SplFixedArray; $obj->$x([1, 2, 3]);
  $x = 'toArray'; $obj = new SplFixedArray; $obj->$x();
  $x = 'SplFixedArray'; new $x();

  $x = 'array_map'; $x($a ==> $a, []);
}

<<__EntryPoint, __DynamicallyCallable>>
function main_builtins() {
  test();
  echo "======================================================\n";
  test();
}

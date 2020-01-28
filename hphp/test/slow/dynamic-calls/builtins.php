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
  $x = 'count'; $x(varray[]);

  $x = 'HH\Vector::fromItems'; $x(varray[]);
  $x = varray['HH\Vector', 'fromItems']; $x(varray[]);
  $x = varray[new Vector, 'fromItems']; $x(varray[]);
  $x = varray[new Vector, 'toArray']; $x();
  $x = 'HH\Vector'; $x::fromItems(varray[]);
  $x = 'fromItems'; Vector::$x(varray[]);

  $x = 'toArray'; $obj = new Vector; $obj->$x();
  $x = 'HH\Vector'; new $x();

  $obj = new A; $obj->test();

  $x = 'SplFixedArray::fromArray'; $x(varray[1, 2, 3]);
  $x = varray['SplFixedArray', 'fromArray']; $x(varray[1, 2, 3]);
  $x = varray[new SplFixedArray, 'fromArray']; $x(varray[1, 2, 3]);
  $x = varray[new SplFixedArray, 'toArray']; $x();
  $x = 'SplFixedArray'; $x::fromArray(varray[1, 2, 3]);
  $x = 'fromArray'; SplFixedArray::$x(varray[1, 2, 3]);

  $x = 'toArray'; $obj = new SplFixedArray; $obj->$x();
  $x = 'SplFixedArray'; new $x();

  $x = 'array_map'; $x($a ==> $a, varray[]);

  $x = null; $x?->foo();
}

<<__EntryPoint, __DynamicallyCallable>>
function main_builtins() {
  test();
  echo "======================================================\n";
  test();
}

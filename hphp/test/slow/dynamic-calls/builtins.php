<?hh

class A extends DateTime {
  public function test() :mixed{
    $x = 'createFromFormat';
    self::$x('j-M-Y', '05-Nov-2017');
    static::$x('j-M-Y', '05-Nov-2017');
    parent::$x('j-M-Y', '05-Nov-2017');
    $x = 'getTimezone'; $this->$x();
  }
}

function test() :mixed{
  $x = 'count'; $x(varray[]);

  $x = 'HH\Vector::fromItems'; $x(varray[]);
  $x = varray['HH\Vector', 'fromItems']; $x(varray[]);
  $x = varray[new Vector, 'fromItems']; $x(varray[]);
  $x = varray[new Vector, 'toVArray']; $x();
  $x = 'HH\Vector'; $x::fromItems(varray[]);
  $x = 'fromItems'; Vector::$x(varray[]);

  $x = 'toVArray'; $obj = new Vector; $obj->$x();
  $x = 'HH\Vector'; new $x();

  $obj = new A; $obj->test();

  $x = 'array_map'; $x($a ==> $a, varray[]);

  $x = null; $x?->foo();
}

<<__EntryPoint, __DynamicallyCallable>>
function main_builtins() :mixed{
  test();
  echo "======================================================\n";
  test();
}

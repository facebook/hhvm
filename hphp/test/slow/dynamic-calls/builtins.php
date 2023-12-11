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
  $x = 'count'; $x(vec[]);

  $x = 'HH\Vector::fromItems'; $x(vec[]);
  $x = vec['HH\Vector', 'fromItems']; $x(vec[]);
  $x = vec[new Vector, 'fromItems']; $x(vec[]);
  $x = vec[new Vector, 'toVArray']; $x();
  $x = 'HH\Vector'; $x::fromItems(vec[]);
  $x = 'fromItems'; Vector::$x(vec[]);

  $x = 'toVArray'; $obj = new Vector; $obj->$x();
  $x = 'HH\Vector'; new $x();

  $obj = new A; $obj->test();

  $x = 'array_map'; $x($a ==> $a, vec[]);

  $x = null; $x?->foo();
}

<<__EntryPoint, __DynamicallyCallable>>
function main_builtins() :mixed{
  test();
  echo "======================================================\n";
  test();
}

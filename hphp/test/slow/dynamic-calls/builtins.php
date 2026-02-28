<?hh

class A extends DateTime {
  public function test() :mixed{
    $x = 'createFromFormat';
    HH\dynamic_class_meth(self::class, $x)('j-M-Y', '05-Nov-2017');
    HH\dynamic_class_meth(static::class, $x)('j-M-Y', '05-Nov-2017');
    HH\dynamic_class_meth(parent::class, $x)('j-M-Y', '05-Nov-2017');
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
  $x = 'fromItems'; HH\dynamic_class_meth(Vector::class, $x)(vec[]);

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

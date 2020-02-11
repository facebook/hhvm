<?hh
 class overloaded {
  private $values;
  function __construct()
  {
    $this->values = darray['a' => 0];
  }
  function __set($name, $value)
  {
    print "set $name = $value ($name was ".$this->values[$name].")\n";
    $this->values[$name] = $value;
  }
  function __get($name)
  {
    print "get $name (returns ".$this->values[$name].")\n";
    return $this->values[$name];
  }
}
<<__EntryPoint>>
function main_entry(): void {

  $test = new overloaded();
  $test->a++;     // __get(), then __set()
  ++$test->a;
}

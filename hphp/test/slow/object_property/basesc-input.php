<?hh

class MyClass {
  protected static $stack = vec[];

  public function addToStack(MyClass $o)
:mixed  {
    self::$stack[] = $o;
  }
}


<<__EntryPoint>>
function main_basesc_input() :mixed{
$obj = new MyClass();
$obj->addToStack($obj);
$obj->addToStack($obj);
echo "Done\n";
}

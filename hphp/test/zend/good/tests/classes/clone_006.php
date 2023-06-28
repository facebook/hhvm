<?hh

class MyCloneable {
  public static $id = 0;

  function __construct() {
    $this->id = self::$id++;
  }

  function __clone() :mixed{
    $this->address = "New York";
    $this->id = self::$id++;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $original = new MyCloneable();

  $original->name = "Hello";
  $original->address = "Tel-Aviv";

  echo $original->id . "\n";

  $clone = clone $original;

  echo $clone->id . "\n";
  echo $clone->name . "\n";
  echo $clone->address . "\n";
}

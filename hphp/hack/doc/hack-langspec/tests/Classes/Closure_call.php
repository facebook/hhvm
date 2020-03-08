// NOT YET IMPLEMENTED IN CHECKER OR ENGINE <?hh // strict

namespace NS_Closure_call;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

class Value {
  protected int $value;

  public function __construct(int $value) {
    $this->value = $value;
  }

  public function getValue(): int {
    return $this->value;
  }
}

function main(): void {

  $three = new Value(3);
  $four = new Value(4);

  $closure = function ($delta) { var_dump($this->getValue() + $delta); };
  $closure->call($three, 4);
  $closure->call($four, 4);
}

/* HH_FIXME[1002] call to main in strict*/
main();

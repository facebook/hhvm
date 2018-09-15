<?hh // strict

/** Function doc block */
function function_call(MyClass $myObject, int $x): void {
  $myObject->methodCall();
  function_call($myObject , $x);
}

class MyClass {
  /** Method doc block */
  public function methodCall(): void {}
}

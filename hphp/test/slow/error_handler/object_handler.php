<?hh
class MyClass {
  public function __construct() {
    // See GitHub #2113
    $this->prop1 = 'something';
    $this->prop2 = 'somethingElse';
    $this->prop3 = 'somethingElseThen';

    var_dump(set_error_handler(vec[$this, 'errorHandler']));
  }

  public function errorHandler($severity, $message, $file = NULL,
                               $line = NULL) :mixed{
    return false;
  }

  public function fail() :mixed{
    user_error('Try to cause an error', E_USER_ERROR);
  }
}

function main() :mixed{
  $x = new MyClass();
  $x->fail();
}


<<__EntryPoint>>
function main_object_handler() :mixed{
main();
}

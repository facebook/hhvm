<?hh

class Functor {
  public function __invoke($x) :mixed{
    return intval($x);
  }
}

class Base {
  private function blah() :mixed{ echo 'Base::blah', "\n"; }
  public function callInScope($x) :mixed{
    $x();
  }
}

class C extends Base {
  private function blah() :mixed{ echo __CLASS__, "\n"; }
  public function exposeBlah() :mixed{
    return vec[$this, 'blah'];
  }

  public static function intval($x) :mixed{
    return intval($x);
  }
  public function inst_intval($x) :mixed{
    return self::intval($x);
  }
}

class Foo extends Base {
  public function get() :mixed{ return vec[$this, 'blah']; }
}

function invoker($x) :mixed{
  call_user_func($x);
  $x();
}

function test_inheritance() :mixed{
  echo ' = ', __FUNCTION__, " =\n";

  $x = new C();
  $x->callInScope($x->exposeBlah());
}

function test_invocation_syntaxes() :mixed{
  echo ' = ', __FUNCTION__, " =\n";

  $test = '10f';
  $call_functor = new Functor();
  $inst = new C();
  $call_func_string = 'intval';
  $call_static_arr = vec['C', 'intval'];
  $call_static_string = 'C::intval';
  $call_instance = vec[$inst, 'inst_intval'];
  $call_static_on_instance = vec[$inst, 'intval'];
  $call_closure = function($x) {return C::intval($x);};
  $call_invalid = vec['C', 'noSuchMethod'];

  echo "* call_user_func ********************\n";
  var_dump(call_user_func($call_func_string, $test));
  var_dump(call_user_func($call_functor, $test));
  var_dump(call_user_func($call_closure, $test));
  var_dump(call_user_func($call_static_string, $test));
  var_dump(call_user_func($call_static_arr, $test));
  var_dump(call_user_func($call_instance, $test));
  var_dump(call_user_func($call_static_on_instance, $test));

  echo "* ()-invoke ********************\n";
  var_dump($call_func_string($test));
  var_dump($call_functor($test));
  var_dump($call_closure($test));
  var_dump($call_static_arr($test));
  var_dump($call_instance($test));
  var_dump($call_static_on_instance($test));

  var_dump($call_invalid($test)); // fatals
  var_dump($call_static_string($test)); // fatals
}

<<__EntryPoint>> function main(): void {
  test_inheritance();
  test_invocation_syntaxes();
}
